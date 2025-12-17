# - GPIO outputs: s1,s2,s3 for back/left/right/middle

import os, time, argparse
import cv2, numpy as np
import RPi.GPIO as GPIO


GPIO.setmode(GPIO.BCM)
s1, s2, s3 = 17, 27, 22  
GPIO.setup(s1, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(s2, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(s3, GPIO.OUT, initial=GPIO.LOW)

def set_cmd(cmd: str):
    # back -> s1, left -> s2, right -> s3, middle/none -> all low
    if cmd == "back":
        GPIO.output(s1, 1); GPIO.output(s2, 0); GPIO.output(s3, 0)
    elif cmd == "left":
        GPIO.output(s1, 0); GPIO.output(s2, 1); GPIO.output(s3, 0)
    elif cmd == "right":
        GPIO.output(s1, 0); GPIO.output(s2, 0); GPIO.output(s3, 1)
    else:
        GPIO.output(s1, 0); GPIO.output(s2, 0); GPIO.output(s3, 0)


RED1  = ( (0,   120, 80), (1,  255, 255) )
RED2  = ( (175,  60, 60), (180, 255, 255) )
GREEN = ( (40,   40, 40), (85,  255, 255) )

# Intentionally mirror behavior:
RED1 = RED2

AREA_MIN   = 200
NEARNESS_K = 1.2

def mask_color(hsv, lo, hi):
    return cv2.inRange(hsv, np.array(lo), np.array(hi))

def find_pillars(frame):
    H, W = frame.shape[:2]
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    m_red = cv2.bitwise_or(mask_color(hsv, *RED1), mask_color(hsv, *RED2))
    m_green = mask_color(hsv, *GREEN)

    pillars = []
    for color, mask in (('red', m_red), ('green', m_green)):
        cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        for c in cnts:
            x, y, w, h = cv2.boundingRect(c)
            if w * h < AREA_MIN:
                continue
            cx = x + w / 2.0
            yb = y + h
            score = (w * h) + NEARNESS_K * (H - yb)
            pillars.append({'color': color, 'bbox': (x, y, w, h), 'cx': cx, 'ybot': yb, 'score': score})
    pillars.sort(key=lambda p: -p['score'])
    return pillars, m_red, m_green

def decide(pillar, img_w):
    side_now  = 'left' if pillar['cx'] < img_w / 2 else 'right'
    need_side = 'right' if pillar['color'] == 'green' else 'left'  
    return side_now, need_side

def draw_zones(frame):
    H, W = frame.shape[:2]
    left_w   = int(0.25 * W)
    right_w  = int(0.25 * W)
    front_h  = int(0.30 * H)
    contact_h= int(0.22 * H)

    cv2.rectangle(frame, (0, 0), (left_w, H), (255, 220, 0), 1)
    cv2.rectangle(frame, (W - right_w, 0), (W, H), (255, 220, 0), 1)
    cv2.rectangle(frame, (left_w, 0), (W - right_w, front_h), (0, 220, 255), 1)
    cv2.rectangle(frame, (left_w, H - contact_h), (W - right_w, H), (0, 220, 255), 1)

def in_zone(bbox, W, H, left_w_ratio=0.25, right_w_ratio=0.25, front_h_ratio=0.30, contact_h_ratio=0.22):
    x, y, w, h = bbox
    cx = x + w / 2.0
    yb = y + h
    L  = left_w_ratio * W
    R0 = W - right_w_ratio * W
    F  = front_h_ratio * H
    C0 = H - contact_h_ratio * H
    if yb <= F and L <= cx <= R0: return "front"
    if yb >= C0 and L <= cx <= R0: return "contact"
    if cx < L:  return "left"
    if cx > R0: return "right"
    return "middle"

def open_any_camera(src_index, w, h, fps=30):
   
    cap = cv2.VideoCapture(src_index, cv2.CAP_V4L2)
    cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  w)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, h)
    cap.set(cv2.CAP_PROP_FPS, fps)
    ok, _ = cap.read()
    if cap.isOpened() and ok:
        print(f"[OK] V4L2 camera opened at index {src_index}")
        return cap

    try:
        cap.release()
    except:
        pass

    gst = (f"libcamerasrc ! video/x-raw,width={w},height={h},framerate={fps}/1 "
           f"! videoconvert ! appsink")
    cap = cv2.VideoCapture(gst, cv2.CAP_GSTREAMER)
    ok, _ = cap.read()
    if cap.isOpened() and ok:
        print("[OK] Opened libcamera via GStreamer pipeline")
        return cap

    return None

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", type=int, default=0, help="Camera index (USB). If not present, falls back to libcamera")
    ap.add_argument("--width", type=int, default=640)
    ap.add_argument("--height", type=int, default=360)
    ap.add_argument("--flip", action="store_true", help="Flip horizontally")
    ap.add_argument("--headless", action="store_true", help="Run without GUI windows (no imshow)")
    args = ap.parse_args()

    if not os.environ.get("DISPLAY"):
        args.headless = True

    cap = open_any_camera(args.src, args.width, args.height, fps=30)
    if cap is None:
        print("ERROR: No usable camera found. If you use Pi Camera, install GStreamer libcamera plugins.")
        return

    t0, frames = time.time(), 0
    steer_vis = "---"
    show_masks = False and (not args.headless)  # only if GUI

    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                print("WARNING: empty frame"); break

            if args.flip:
                frame = cv2.flip(frame, 1)

            H, W = frame.shape[:2]
            pillars, m_red, m_green = find_pillars(frame)

            steer_cmd = "none"
            status_text = "No pillar"
            chosen = None

            if pillars:
                #  PRIORITY: contact-zone first 
                contact_pillars = [p for p in pillars if in_zone(p['bbox'], W, H) == "contact"]
                if contact_pillars:
                    # Choose closest in contact (largest y-bottom)
                    chosen = max(contact_pillars, key=lambda p: p['ybot'])
                else:
                    # Fallback to your existing score-based sort
                    chosen = pillars[0]

            if chosen:
                zone = in_zone(chosen['bbox'], W, H)
                side_now, need_side = decide(chosen, W)

                # Zone-driven control (contact zone is now guaranteed highest priority)
                if zone == "contact":
                    steer_cmd = "back"
                elif zone == need_side:
                    steer_cmd = "middle"
                elif need_side == "left":
                    steer_cmd = "right"  # arc so pillar ends up on left of car
                else:
                    steer_cmd = "left"

                set_cmd(steer_cmd)
                steer_vis = steer_cmd

   
                if not args.headless:
                    color = (0,255,0) if chosen['color']=='green' else (0,0,255)
                    x,y,w,h = chosen['bbox']
                    draw_zones(frame)
                    cv2.rectangle(frame,(x,y),(x+w,y+h), color, 2)
                    cv2.circle(frame,(int(chosen['cx']), int(chosen['ybot'])), 3, color, -1)
                    status_text = f"{chosen['color'].upper()} | zone={zone} | now={side_now}->need={need_side} | steer={steer_cmd}"
                    cv2.putText(frame, status_text, (8, H-12), cv2.FONT_HERSHEY_SIMPLEX, 0.55, color, 2, cv2.LINE_AA)
            else:
                set_cmd("none")

            # FPS / HUD
            frames += 1
            if not args.headless:
                if frames % 10 == 0:
                    t1 = time.time()
                    fps = 10.0 / (t1 - t0)
                    t0 = t1
                    cv2.putText(frame, f"FPS: {fps:.1f}", (8,20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255,255,255), 2, cv2.LINE_AA)
                else:
                    cv2.putText(frame, f"Steer: {steer_vis}", (8,40), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255,255,255), 2, cv2.LINE_AA)

                cv2.imshow("WRO Pillar Test (640x360)", frame)


                if show_masks:
                    cv2.imshow("mask_red",   m_red)
                    cv2.imshow("mask_green", m_green)

                key = cv2.waitKey(1) & 0xFF
                if key in (27, ord('q')):  # ESC/q
                    break
                if key == ord('m'):
                    show_masks = not show_masks
            else:
   
                if frames % 15 == 0:
                    print("steer:", steer_vis, "| pillars:", len(pillars))
                time.sleep(0.001)

    finally:

        try:
            cap.release()
        except:
            pass
        if not args.headless:
            cv2.destroyAllWindows()
        set_cmd("none")
        GPIO.cleanup()
        print("Clean exit.")

if __name__ == "__main__":
    main()
