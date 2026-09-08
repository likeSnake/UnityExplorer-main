# ExternalResolve202x 部署与学习指南

欢迎！本指南将带你从零开始部署并运行这个 Unity 逆向工程学习项目。即使你是编程新手，只要按照步骤操作，也能顺利跑通。

---

## 第一步：准备开发环境

要运行这个 C++ 项目，你需要微软的 Visual Studio。

1.  **下载 Visual Studio 2022**
    *   访问官网：[visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
    *   下载 **Community 2022** 版本（社区版，个人学习免费）。

2.  **安装配置**
    *   运行安装程序。
    *   在"工作负载" (Workloads) 选项卡中，**必须勾选**：
        *   ✅ **使用 C++ 的桌面开发** (Desktop development with C++)
    *   点击安装并等待完成。

---

## 第二步：下载核心依赖 (GLM)

本项目依赖一个名为 GLM 的数学库来处理 3D 坐标运算。你需要手动下载它。

1.  **下载**
    *   访问 GitHub 发布页：[https://github.com/g-truc/glm/tags](https://github.com/g-truc/glm/tags)
    *   下载最新版本的 ZIP 包 (例如 `glm-1.0.1-light.zip` 或完整版 `glm-x.x.x.zip`)。

2.  **解压与放置**
    *   找到你在硬盘上的本项目文件夹。
    *   在根目录下创建一个名为 `deps` 的文件夹（如果还没有的话）。
    *   把下载的压缩包可以在 `deps` 下解压。
    *   **关键检查**：
        确保你的文件夹结构看起来是这样的（不要多套一层文件夹）：
        ```
        Unity202x-eXternalrEsolve/
        ├── App/
        ├── include/
        └── deps/
            └── glm/
                └── glm/          <-- 里面应该能看到 detail, mat4x4.hpp 等文件
        ```
        *也就是说，`deps/glm` 里面直接就是 GLM 的源码头文件。*

---

## 第三步：打开项目

为了方便你使用，我已经为你准备好了现成的 Visual Studio 解决方案。

1.  进入 `App` 文件夹。
2.  双击运行 **`ExternalResolveConsole.sln`**。
3.  Visual Studio 会启动并加载项目。

---

## 第四步：编译与运行

1.  **设置编译模式**
    *   在 VS 顶部工具栏，你会看到两个下拉框。
    *   确保设置为：**Debug** (或 Release) 和 **x64**。
    *   **注意**：只能选 **x64**，因为现在的电脑游戏基本都是 64 位的。

2.  **生成解决方案**
    *   点击菜单栏的 `生成 (Build)` -> `生成解决方案 (Build Solution)`。
    *   或者直接按快捷键 **F7** (或 Ctrl+Shift+B)。
    *   底部的"输出"窗口如果显示 `成功 1 个，失败 0 个`，恭喜你，编译成功了！

3.  **运行程序**
    *   按 **F5** 开始调试运行。
    *   你会看到一个黑色的控制台窗口弹出。

---

## 常见问题 (FAQ)

**Q: 运行后黑框一闪而过报错？**
A: 这是正常的。因为这个工具需要读取正在运行的 Unity 游戏内存。
*   你需要先打开一个 Unity 制作的游戏（例如 *Among Us*, *鬼谷八荒* 等）。
*   然后再运行本程序。
*   程序会自动尝试寻找 Unity 进程并读取数据。

**Q: 提示找不到 `glm/glm.hpp`？**
A: 请回头仔细检查 **第二步**。通常是因为你解压 GLM 时多套了一层文件夹。
*   错误示范：`deps/glm-1.0.1/glm/glm/`
*   正确示范：`deps/glm/glm/` (确保 VS 项目里包含路径能对应上)

---

## 学习建议

现在你已经跑通了环境，建议按以下顺序阅读代码：

1.  打开 `App/ExternalResolveConsole/main.cpp`：这是入口，看它是如何调用库函数的。
2.  按住 `Ctrl` 点击 `er2::AutoInit()`：进入库的内部，看看它是如何扫描进程的。
3.  尝试修改 `main.cpp`，比如让它每隔 1 秒打印一次摄像机坐标。

祝你学习愉快！

---

## 进阶：不想打开 VS？在当前 IDE 中编译

如果你不想每次都打开臃肿 Visual Studio 界面，或者你想直接在当前的 **Antigravity IDE** 中编译运行：

1.  **前提**：你仍然需要安装 Visual Studio (或 VS Build Tools)，因为我们依赖它的 C++ 编译器 (cl.exe)。
2.  **一键运行**：
    *   在当前 IDE 打开终端 (Terminal)。
    *   确保你在项目根目录。
    *   输入并运行脚本：`.\BuildAndRun.bat`
3.  **效果**：
    *   脚本会自动找到 VS 编译器环境。
    *   自动编译代码。
    *   自动运行生成的 exe 程序。
    *   **全程无需离开当前编辑器！**
