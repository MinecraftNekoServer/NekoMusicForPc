# Neko云音乐 PC版
![](https://count.getloli.com/get/@:NekoMusicPC?theme=moebooru)

> [!TIP]
> 🐾 **移动端入口**：[点击这里查看 Neko云音乐 安卓版仓库](https://github.com/MinecraftNekoServer/NekoMusicForAndroid)

---

## 🛠️ 构建指南

### 前置要求

| 依赖 | 最低版本 |
| --- | --- |
| CMake | ≥ 3.20 |
| Qt 6 | ≥ 6.2 |
| C++17 编译器 | GCC ≥ 9 / MSVC 2019 / Clang ≥ 10 |

**安装依赖：**

```bash
sudo apt install cmake qt6-base-dev qt6-multimedia-dev
```


### 配置

项目使用 CMake Presets 管理多平台构建，可直接选择对应预设：

```bash
# Linux
cmake --preset linux-debug     # 开发调试
cmake --preset linux-release   # 发布构建

# Windows
cmake --preset windows-debug
cmake --preset windows-release

# macOS
cmake --preset macos-debug
cmake --preset macos-release
```

也可以手动配置：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
```

### 编译

```bash
# 使用预设构建
cmake --build build/linux-debug -j$(nproc)

# 或手动指定目录
cmake --build build --parallel
```

### 运行测试

```bash
ctest --test-dir build/linux-debug --output-on-failure
```

### 安装

```bash
cmake --install build/linux-release --prefix /usr/local
```

---

## 🛠️ 构建指南 (old旧版本 Electron)

想要亲手捕捉这只“云音乐猫”吗？按照以下步骤操作就可以啦，喵！

### 1. 前置要求

首先，请确保你的电脑里住着 **Node.js 22** 版本。你可以通过以下
命令检查喵：

```bash
node -v  # 确认输出为 v22.x.x

```

### 2. 获取与准备

打开你的终端（或者命令提示符），跳进项目的怀抱：

```bash
# 进入项目路径
cd NekoMusicForPC

```

### 3. 安装依赖

让猫猫吃饱（安装必要的库文件）：

```bash
npm install

```

### 4. 执行构建

可以根据正在使用的平台，选择对应的构建指令喵：

| 目标平台 | 执行指令 |
| --- | --- |
| **Windows** | `npm run build:win` |
| **Linux** | `npm run build:linux` |
| **macOS** | `npm run build:mac` |

---

## 🎵 功能特性

### 音乐播放
- 支持在线音乐播放
- 显示当前播放歌曲信息（标题、艺术家）
- 播放进度控制
- 音量调节
- 系统托盘支持（最小化到托盘）

### 音乐搜索
- 实时搜索音乐
- 显示搜索结果
- 点击搜索结果直接播放

### 音乐浏览
- 热门音乐推荐
- 最新音乐列表
- 歌单浏览
- 音乐分类

### 用户功能
- 用户登录/注册
- 收藏音乐
- 播放历史
- 个人歌单

### 其他功能
- 多语言支持
- 主题切换
- 响应式UI设计
- 无边框窗口

## 📂 构建产物

构建成功后，你可以在项目根目录下的 `dist` (或指定的输出文件夹) 中找到打包好的可执行文件喵！

## 🤝 贡献与反馈

如果你在构建过程中踩到了猫尾巴（遇到错误），欢迎提交 **Issue** 或发起 **Pull Request**！喵呜～

---