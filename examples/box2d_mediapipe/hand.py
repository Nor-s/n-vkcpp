import cv2
import sys
import mediapipe as mp
# shared memory
import mmap
import struct
import time

if sys.platform != 'win32':
    import sysv_ipc

if __name__ == '__main__':
    shm = None
    time.sleep(2)
    if sys.platform != 'win32':
        shm = sysv_ipc.SharedMemory(777)

    # 랜드마크를 그리기 위한 변수
    mp_drawing = mp.solutions.drawing_utils
    # 손을 처리하기 위한 변수
    mp_drawing_styles = mp.solutions.drawing_styles
    # 랜드마크 표시 스타일
    mp_hands = mp.solutions.hands

    # For webcam input:
    cap = cv2.VideoCapture(0)
    # width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
    # height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
    # print('original size: %d, %d' % (width, height))

    # cap.set(cv2.CAP_PROP_FRAME_WIDTH, width/3)
    # cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height/3)

    # width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
    # height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
    # print('changed size: %d, %d' % (width, height))

    with mp_hands.Hands(
        model_complexity=1,
        max_num_hands=1,
        # 손가락 감지 최소 신뢰성(1.0에 가까울수록 신뢰성이 높은 것만 인식)
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5   # 손가락 추적 최소 신뢰성(1.0에 가까울수록 추적율이 높은 것만 인식)
    ) as hands:
        while cap.isOpened():
            success, image = cap.read()
            if not success:
                print("Ignoring empty camera frame.")
                # If loading a video, use 'break' instead of 'continue'.
                continue

            # To improve performance, optionally mark the image as not writeable to
            # pass by reference.
            image.flags.writeable = False
            # 사진을 좌/우 반전 시킨후 BGR에서 RGB로 변환
            if sys.platform != 'win32':
                image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            else:
                image = cv2.cvtColor(cv2.flip(image, 1), cv2.COLOR_BGR2RGB)

            # 손 인식 결과
            results = hands.process(image)

            # Draw the hand annotations on the image.
            image.flags.writeable = True
            # 이미지를 다시 RGB에서 BGR로 변경
            image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
            if results.multi_hand_landmarks:
                for hand_landmarks in results.multi_hand_landmarks:
                    if sys.platform == 'win32':
                        # ipc 공유메모리를 위한 변수
                        shm = mmap.mmap(0, 512, tagname="Local\\MPipe")
                        if shm:
                            for idx in range(0, 21):
                                shm.write(struct.pack(
                                    'f', hand_landmarks.landmark[idx].x))
                                shm.write(struct.pack(
                                    'f', hand_landmarks.landmark[idx].y))
                                shm.write(struct.pack(
                                    'f', hand_landmarks.landmark[idx].z))
                    else:
                        if shm:
                            offset = 0
                            for idx in range(0, 21):
                                shm.write(struct.pack(
                                    'f', (1.0 - hand_landmarks.landmark[idx].x)), offset)
                                offset += 4
                                shm.write(struct.pack(
                                    'f',  hand_landmarks.landmark[idx].y), offset)
                                offset += 4
                                shm.write(struct.pack(
                                    'f', hand_landmarks.landmark[idx].z), offset)
                                offset += 4

                    mp_drawing.draw_landmarks(
                        image,
                        hand_landmarks,
                        mp_hands.HAND_CONNECTIONS,
                        mp_drawing_styles.get_default_hand_landmarks_style(),
                        mp_drawing_styles.get_default_hand_connections_style())
                    if sys.platform != 'win32':
                        cv2.imshow('MediaPipe Hands', cv2.flip(image, 1))
                    else:
                        cv2.imshow('MediaPipe Hands', image)

            if cv2.waitKey(5) & 0xFF == 27:
                break
    cap.release()
