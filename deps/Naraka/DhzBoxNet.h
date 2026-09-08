#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

//#include "kmboxNet.cpp"
#include <winsock2.h>
//#pragma comment(lib, "ws2_32.lib")



class DHZBOX {
    bool connected;  // 保存初始化是否成功
    SOCKADDR_IN addrSrv;
public:
    
    static bool Init(const std::string& ip, int port, int deviceIndex = 0) {
        std::unique_ptr<DHZBOX>& instance = Instance();
        if (!instance) {
            instance.reset(new DHZBOX(ip, port, deviceIndex));
            std::cout << "IP: " << instance->IP << " --Port: " << instance->PORT << std::endl;
        }
        if (!instance->isConnected()) {
            std::cerr << "DHZBOX 初始化失败" << std::endl;
        }
        return instance->isConnected();
    }

    static DHZBOX& getInstance() {
        std::unique_ptr<DHZBOX>& instance = Instance();
        if (!instance) {
            std::cerr << "对象为空" << std::endl;
            static DHZBOX fallback("127.0.0.1", 9000, 0);
            return fallback;
        }
        std::cout << "IP: " << instance->IP << " --Port: " << instance->PORT << std::endl;
        return *instance;
    }

    bool isConnected() const { return connected; }

public:
    static std::unique_ptr<DHZBOX>& Instance() {
        static std::unique_ptr<DHZBOX> instance;
        return instance;
    }

    DHZBOX(std::string IP, int PORT, int RANDOM)
        : IP(IP), PORT(PORT), RANDOM(RANDOM), RECEIVER_FLAG(false), LEFTSTATE(0), RIGHTSTATE(0), MIDDLESTATE(0), SIDE1STATE(0), SIDE2STATE(0), KEYSTATE("") {
        // 初始化 Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            connected = false;
            //return;
        }
        std::string test_message = encrypt_string("move(0,0)");
        if (udp_sender(test_message)) {
            connected = true;
        }
        else {
            WSACleanup();
            connected = false;
        }
    }


    // 禁止拷贝与赋值
    DHZBOX(const DHZBOX&) = delete;
    DHZBOX& operator=(const DHZBOX&) = delete;

    ~DHZBOX() {
        WSACleanup();
    }

public:
    std::string IP;
    int PORT;
    int RANDOM;
    bool RECEIVER_FLAG;
    int LEFTSTATE, RIGHTSTATE, MIDDLESTATE, SIDE1STATE, SIDE2STATE;
    std::string KEYSTATE;

    // 加密字符串
    std::string encrypt_string(const std::string& str) {
        std::string encrypted_string;
        for (char ch : str) {
            if (isalpha(ch)) {
                if (islower(ch)) {
                    encrypted_string += char((ch - 'a' + RANDOM) % 26 + 'a');
                }
                else if (isupper(ch)) {
                    encrypted_string += char((ch - 'A' + RANDOM) % 26 + 'A');
                }
            }
            else {
                encrypted_string += ch;
            }
        }
        return encrypted_string;
    }



    int udp_sender(const std::string& message) {
        SOCKET SCOK_sender = socket(AF_INET, SOCK_DGRAM, 0);
        if (SCOK_sender == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return 0;
        }

        // 设置接收超时时间为 50 毫秒
        DWORD timeout = 50; // 
        if (setsockopt(SCOK_sender, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
            std::cerr << "Failed to set socket timeout: " << WSAGetLastError() << std::endl;
            closesocket(SCOK_sender);
            return 0;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(IP.c_str());

        std::cout << "IP: " << IP << " --Port: " << -PORT << std::endl;
        // 发送数据
        int result = sendto(SCOK_sender, message.c_str(), message.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
        if (result == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(SCOK_sender);
            return 0;
        }

        // 接收回应
        char recvBuf[1024];
        sockaddr_in recv_addr;
        int addr_len = sizeof(recv_addr);
        int recv_result = recvfrom(SCOK_sender, recvBuf, sizeof(recvBuf), 0, (sockaddr*)&recv_addr, &addr_len);
        if (recv_result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                //std::cerr << "Receive timed out: No response from server." << std::endl;
            }
            else {
                //std::cerr << "Receive failed: " << error << std::endl;
            }
            closesocket(SCOK_sender);
            return 0;
        }

        recvBuf[recv_result] = '\0';  // 结束字符串
        //std::cout << "Received response: " << recvBuf << std::endl;

        closesocket(SCOK_sender);
        return 1; // 成功
    }

    // UDP 接收线程
    void udp_receiver(int port) {
        SOCKET SCOK_receiver = socket(AF_INET, SOCK_DGRAM, 0);
        if (SCOK_receiver == INVALID_SOCKET) {
            std::cerr << "Receiver socket creation failed" << std::endl;
            return;
        }

        sockaddr_in recv_addr;
        recv_addr.sin_family = AF_INET;
        recv_addr.sin_port = htons(port);
        recv_addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(SCOK_receiver, (sockaddr*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR) {
            std::cerr << "Binding failed" << std::endl;
            return;
        }


        while (RECEIVER_FLAG) {
            char buffer[1024];
            sockaddr_in sender_addr;
            int sender_len = sizeof(sender_addr);
            int result = recvfrom(SCOK_receiver, buffer, sizeof(buffer), 0, (sockaddr*)&sender_addr, &sender_len);
            if (result == SOCKET_ERROR) {
                std::cerr << "Receive failed" << std::endl;
                continue;
            }

            buffer[result] = '\0';
            std::string msg(buffer);
            //std::cout << "Received data: " << msg << std::endl;

            // 处理鼠标状态
            std::stringstream ss(msg);
            std::string temp;
            try {
                std::getline(ss, temp, '|'); LEFTSTATE = std::stoi(temp);
                std::getline(ss, temp, '|'); MIDDLESTATE = std::stoi(temp);
                std::getline(ss, temp, '|'); RIGHTSTATE = std::stoi(temp);
                std::getline(ss, temp, '|'); SIDE1STATE = std::stoi(temp);
                std::getline(ss, temp, '|'); SIDE2STATE = std::stoi(temp);
                std::getline(ss, temp, '|'); KEYSTATE = temp;
            }
            catch (...) {
                LEFTSTATE = RIGHTSTATE = MIDDLESTATE = SIDE1STATE = SIDE2STATE = 0;
                KEYSTATE = "";
            }
        }

        closesocket(SCOK_receiver);
    }
    // 二阶贝塞尔曲线运动
    void move_bezier(int x, int y, int ms, int x1, int y1) {
        // 贝塞尔曲线的起始点 P0（当前位置）
       /* int P0_x = softmouse.x;
        int P0_y = softmouse.y;

        // 目标点 P2（目标点）
        int P2_x = x;
        int P2_y = y;

        // 控制点 P1
        int P1_x = x1;
        int P1_y = y1;

        // 贝塞尔曲线的时间参数 t
        const int num_steps = 100;  // 步数（越大曲线越平滑）
        int delay_ms = ms / num_steps;  // 每步之间的时间，确保总时长为 ms

        for (int step = 0; step <= num_steps; ++step) {
            float t = step / (float)num_steps;

            // 计算贝塞尔曲线坐标
            int current_x = (int)((1 - t) * (1 - t) * P0_x + 2 * (1 - t) * t * P1_x + t * t * P2_x);
            int current_y = (int)((1 - t) * (1 - t) * P0_y + 2 * (1 - t) * t * P1_y + t * t * P2_y);

            // 更新鼠标位置
            std::string cmd = encrypt_string("move(" + std::to_string(current_x) + "," + std::to_string(current_y) + ")");
            udp_sender(cmd);

            // 等待一小段时间以模拟平滑的运动
            //std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }*/
    }

private:
    struct KeyNameEntry {
        uint32_t code;
        const char* name;
    };

    static const char* KeyCodeToName(uint32_t key) {
        static const KeyNameEntry kKeyNames[] = {
        {0x00u, "KEY_NONE"},
        {0x01u, "KEY_ERR_OVF"},
        {0x04u, "KEY_A"},
        {0x05u, "KEY_B"},
        {0x06u, "KEY_C"},
        {0x07u, "KEY_D"},
        {0x08u, "KEY_E"},
        {0x09u, "KEY_F"},
        {0x0Au, "KEY_G"},
        {0x0Bu, "KEY_H"},
        {0x0Cu, "KEY_I"},
        {0x0Du, "KEY_J"},
        {0x0Eu, "KEY_K"},
        {0x0Fu, "KEY_L"},
        {0x10u, "KEY_M"},
        {0x11u, "KEY_N"},
        {0x12u, "KEY_O"},
        {0x13u, "KEY_P"},
        {0x14u, "KEY_Q"},
        {0x15u, "KEY_R"},
        {0x16u, "KEY_S"},
        {0x17u, "KEY_T"},
        {0x18u, "KEY_U"},
        {0x19u, "KEY_V"},
        {0x1Au, "KEY_W"},
        {0x1Bu, "KEY_X"},
        {0x1Cu, "KEY_Y"},
        {0x1Du, "KEY_Z"},
        {0x1Eu, "KEY_1"},
        {0x1Fu, "KEY_2"},
        {0x20u, "KEY_3"},
        {0x21u, "KEY_4"},
        {0x22u, "KEY_5"},
        {0x23u, "KEY_6"},
        {0x24u, "KEY_7"},
        {0x25u, "KEY_8"},
        {0x26u, "KEY_9"},
        {0x27u, "KEY_0"},
        {0x28u, "KEY_ENTER"},
        {0x29u, "KEY_ESC"},
        {0x2Au, "KEY_BACKSPACE"},
        {0x2Bu, "KEY_TAB"},
        {0x2Cu, "KEY_SPACE"},
        {0x2Du, "KEY_MINUS"},
        {0x2Eu, "KEY_EQUAL"},
        {0x2Fu, "KEY_LEFTBRACE"},
        {0x30u, "KEY_RIGHTBRACE"},
        {0x31u, "KEY_BACKSLASH"},
        {0x32u, "KEY_HASHTILDE"},
        {0x33u, "KEY_SEMICOLON"},
        {0x34u, "KEY_APOSTROPHE"},
        {0x35u, "KEY_GRAVE"},
        {0x36u, "KEY_COMMA"},
        {0x37u, "KEY_DOT"},
        {0x38u, "KEY_SLASH"},
        {0x39u, "KEY_CAPSLOCK"},
        {0x3Au, "KEY_F1"},
        {0x3Bu, "KEY_F2"},
        {0x3Cu, "KEY_F3"},
        {0x3Du, "KEY_F4"},
        {0x3Eu, "KEY_F5"},
        {0x3Fu, "KEY_F6"},
        {0x40u, "KEY_F7"},
        {0x41u, "KEY_F8"},
        {0x42u, "KEY_F9"},
        {0x43u, "KEY_F10"},
        {0x44u, "KEY_F11"},
        {0x45u, "KEY_F12"},
        {0x46u, "KEY_SYSRQ"},
        {0x47u, "KEY_SCROLLLOCK"},
        {0x48u, "KEY_PAUSE"},
        {0x49u, "KEY_INSERT"},
        {0x4Au, "KEY_HOME"},
        {0x4Bu, "KEY_PAGEUP"},
        {0x4Cu, "KEY_DELETE"},
        {0x4Du, "KEY_END"},
        {0x4Eu, "KEY_PAGEDOWN"},
        {0x4Fu, "KEY_RIGHT"},
        {0x50u, "KEY_LEFT"},
        {0x51u, "KEY_DOWN"},
        {0x52u, "KEY_UP"},
        {0x53u, "KEY_NUMLOCK"},
        {0x54u, "KEY_KPSLASH"},
        {0x55u, "KEY_KPASTERISK"},
        {0x56u, "KEY_KPMINUS"},
        {0x57u, "KEY_KPPLUS"},
        {0x58u, "KEY_KPENTER"},
        {0x59u, "KEY_KP1"},
        {0x5Au, "KEY_KP2"},
        {0x5Bu, "KEY_KP3"},
        {0x5Cu, "KEY_KP4"},
        {0x5Du, "KEY_KP5"},
        {0x5Eu, "KEY_KP6"},
        {0x5Fu, "KEY_KP7"},
        {0x60u, "KEY_KP8"},
        {0x61u, "KEY_KP9"},
        {0x62u, "KEY_KP0"},
        {0x63u, "KEY_KPDOT"},
        {0x64u, "KEY_102ND"},
        {0x65u, "KEY_COMPOSE"},
        {0x66u, "KEY_POWER"},
        {0x67u, "KEY_KPEQUAL"},
        {0x68u, "KEY_F13"},
        {0x69u, "KEY_F14"},
        {0x6Au, "KEY_F15"},
        {0x6Bu, "KEY_F16"},
        {0x6Cu, "KEY_F17"},
        {0x6Du, "KEY_F18"},
        {0x6Eu, "KEY_F19"},
        {0x6Fu, "KEY_F20"},
        {0x70u, "KEY_F21"},
        {0x71u, "KEY_F22"},
        {0x72u, "KEY_F23"},
        {0x73u, "KEY_F24"},
        {0x74u, "KEY_OPEN"},
        {0x75u, "KEY_HELP"},
        {0x76u, "KEY_PROPS"},
        {0x77u, "KEY_FRONT"},
        {0x78u, "KEY_STOP"},
        {0x79u, "KEY_AGAIN"},
        {0x7Au, "KEY_UNDO"},
        {0x7Bu, "KEY_CUT"},
        {0x7Cu, "KEY_COPY"},
        {0x7Du, "KEY_PASTE"},
        {0x7Eu, "KEY_FIND"},
        {0x7Fu, "KEY_MUTE"},
        {0x80u, "KEY_VOLUMEUP"},
        {0x81u, "KEY_VOLUMEDOWN"},
        {0x85u, "KEY_KPCOMMA"},
        {0x87u, "KEY_RO"},
        {0x88u, "KEY_KATAKANAHIRAGANA"},
        {0x89u, "KEY_YEN"},
        {0x8Au, "KEY_HENKAN"},
        {0x8Bu, "KEY_MUHENKAN"},
        {0x8Cu, "KEY_KPJPCOMMA"},
        {0x90u, "KEY_HANGEUL"},
        {0x91u, "KEY_HANJA"},
        {0x92u, "KEY_KATAKANA"},
        {0x93u, "KEY_HIRAGANA"},
        {0x94u, "KEY_ZENKAKUHANKAKU"},
        {0xB6u, "KEY_KPLEFTPAREN"},
        {0xB7u, "KEY_KPRIGHTPAREN"},
        {0xE0u, "KEY_LEFTCTRL"},
        {0xE1u, "KEY_LEFTSHIFT"},
        {0xE2u, "KEY_LEFTALT"},
        {0xE3u, "KEY_LEFTMETA"},
        {0xE4u, "KEY_RIGHTCTRL"},
        {0xE5u, "KEY_RIGHTSHIFT"},
        {0xE6u, "KEY_RIGHTALT"},
        {0xE7u, "KEY_RIGHTMETA"},
        {0xE8u, "KEY_MEDIA_PLAYPAUSE"},
        {0xE9u, "KEY_MEDIA_STOPCD"},
        {0xEAu, "KEY_MEDIA_PREVIOUSSONG"},
        {0xEBu, "KEY_MEDIA_NEXTSONG"},
        {0xECu, "KEY_MEDIA_EJECTCD"},
        {0xEDu, "KEY_MEDIA_VOLUMEUP"},
        {0xEEu, "KEY_MEDIA_VOLUMEDOWN"},
        {0xEFu, "KEY_MEDIA_MUTE"},
        {0xF0u, "KEY_MEDIA_WWW"},
        {0xF1u, "KEY_MEDIA_BACK"},
        {0xF2u, "KEY_MEDIA_FORWARD"},
        {0xF3u, "KEY_MEDIA_STOP"},
        {0xF4u, "KEY_MEDIA_FIND"},
        {0xF5u, "KEY_MEDIA_SCROLLUP"},
        {0xF6u, "KEY_MEDIA_SCROLLDOWN"},
        {0xF7u, "KEY_MEDIA_EDIT"},
        {0xF8u, "KEY_MEDIA_SLEEP"},
        {0xF9u, "KEY_MEDIA_COFFEE"},
        {0xFAu, "KEY_MEDIA_REFRESH"},
        {0xFBu, "KEY_MEDIA_CALC"},
        };
        for (const auto& entry : kKeyNames) {
            if (entry.code == key) {
                return entry.name;
            }
        }
        return nullptr;
    }

public:

    template<typename... Args>
    void MyLogTrue(Args&&... args) {
        (std::cout << ... << args) << std::endl;
    }

    void move(int x, int y) {
        std::string cmd = encrypt_string("move(" + std::to_string(x) + "," + std::to_string(y) + ")");
        udp_sender(cmd);


    }

    void move_with_bezier(int x, int y, int ms, int x1, int y1) {
        move_bezier(x, y, ms, x1, y1);
    }



    void left(int state) {
        std::string cmd = encrypt_string("left(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void right(int state) {
        std::string cmd = encrypt_string("right(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void middle(int state) {
        std::string cmd = encrypt_string("middle(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    // 0释放 1按下
    void side1(int state) {
        std::string cmd = encrypt_string("side1(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    // 0释放 1按下
    void side2(int state) {
        std::string cmd = encrypt_string("side2(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void wheel(int state) {
        std::string cmd = encrypt_string("wheel(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void mouse(int button, int x, int y, int w) {
        std::string cmd = encrypt_string("mouse(" + std::to_string(button) + "," + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(w) + ")");
        udp_sender(cmd);
    }

    void monitor(int port) {
        if (port == 0) {
            std::string cmd = encrypt_string("monitor(0)");
            udp_sender(cmd);
            RECEIVER_FLAG = false;
        }
        else {
            std::string cmd = encrypt_string("monitor(" + std::to_string(port) + ")");
            udp_sender(cmd);
            RECEIVER_FLAG = true;
            std::thread receiver(&DHZBOX::udp_receiver, this, port);
            receiver.detach();
        }
    }

    void KeyDown(const std::string& KEY)
    {
        std::string cmd = "keydown('" + KEY + "')";
        MyLogTrue("发送指令:", cmd);
        cmd = encrypt_string(cmd);
        if (udp_sender(cmd) == 1) {
            //MyLogTrue("成功发送按键按下");
        }
    }

    void KeyUp(const std::string& KEY)
    {
        std::string cmd = "keyup('" + KEY + "')";
        cmd = encrypt_string(cmd);
        if (udp_sender(cmd) == 1) {
            //MyLogTrue("成功发送按键弹起");
        }
    }

    void KeyDown(uint32_t key) {
        const char* key_name = KeyCodeToName(key);
        if (key_name) {
            KeyDown(std::string(key_name));
            return;
        }
        KeyDown(std::to_string(key));
    }

    void KeyUp(uint32_t key) {
        const char* key_name = KeyCodeToName(key);
        if (key_name) {
            KeyUp(std::string(key_name));
            return;
        }
        KeyUp(std::to_string(key));
    }
    

    bool isdown_left()
    {
        return LEFTSTATE;
    }
    bool isdown_middle() { return MIDDLESTATE; }
    bool isdown_right() { return RIGHTSTATE; }
    bool isdown_side1() { return SIDE1STATE; }
    bool isdown_side2() { return SIDE2STATE; }
    std::string isdown() { return KEYSTATE; }

    void mask_left(int state) {
        std::string cmd = encrypt_string("mask_left(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void mask_right(int state) {
        std::string cmd = encrypt_string("mask_right(" + std::to_string(state) + ")");
        udp_sender(cmd);
    }

    void dismask_keyboard(const std::string& key) {
        std::string cmd = encrypt_string("dismask_keyboard(" + key + ")");
        udp_sender(cmd);
    }

    void dismask_keyboard_all() {
        std::string cmd = encrypt_string("dismask_keyboard_all()");
        udp_sender(cmd);
    }
};
