#!/bin/bash

# =========================================
# Neko云音乐 macOS Intel x86 构建脚本
# =========================================

set -e  # 遇到错误立即退出

echo "========================================="
echo "  Neko云音乐 macOS Intel x86 Build Script"
echo "========================================="

# 检查是否在正确的目录
if [ ! -f "/old/package.json" ]; then
    echo "错误：未找到 old/package.json"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 检查是否在macOS系统上运行
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "警告：此脚本建议在macOS系统上运行"
    echo "当前系统: $OSTYPE"
    read -p "是否继续？(y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# 检查CPU架构
CPU_ARCH=$(uname -m)
echo "CPU架构: $CPU_ARCH"

if [[ "$CPU_ARCH" == "arm64" ]]; then
    echo "检测到Apple Silicon (ARM64) 处理器"
    echo "注意：将在Rosetta 2下构建Intel x86版本"
fi

# 进入旧版本目录
cd old

# 检查Node.js版本
NODE_VERSION=$(node -v | cut -d'v' -f2)
echo "Node.js 版本: $NODE_VERSION"

# 检查依赖是否已安装
if [ ! -d "node_modules" ]; then
    echo "未找到 node_modules，正在安装依赖..."
    npm install
fi

echo ""
echo "开始构建 macOS Intel x86 版本..."

# 设置环境变量强制使用x86架构
export npm_config_arch=x64
export ELECTRON_OVERRIDE_DIST_PATH="https://github.com/electron/electron/releases/download/v37.1.0/electron-v37.1.0-darwin-x64.zip"

# 清理之前的构建
if [ -d "dist" ]; then
    echo "清理之前的构建..."
    rm -rf dist
fi

if [ -d "dist-electron" ]; then
    echo "清理之前的Electron构建..."
    rm -rf dist-electron
fi

if [ -d "release" ]; then
    echo "清理之前的发布版本..."
    rm -rf release
fi

# 构建步骤
echo "1. 构建Vite应用..."
npm run build

echo "2. 复制preload文件..."
cp electron/preload.cjs dist-electron/preload.cjs

echo "3. 构建macOS Intel x86应用..."
# 强制使用x64架构构建
# 在Linux上构建macOS应用时使用--dir选项避免dmg构建
if [[ "$OSTYPE" == "darwin"* ]]; then
    npx electron-builder --mac --x64
else
    echo "在Linux上构建macOS应用，使用--dir选项生成未打包的应用"
    npx electron-builder --mac --x64 --dir
fi

echo ""
echo "========================================="
echo "  构建完成！"
echo "========================================="

# 显示构建结果
if [ -d "release" ]; then
    echo "构建产物位于: old/release/"
    
    # 查找dmg文件
    DMG_FILE=$(find release -name "*.dmg" | head -1)
    if [ -n "$DMG_FILE" ]; then
        echo ""
        echo "✅ 成功生成: $(basename "$DMG_FILE")"
        echo "文件大小: $(du -h "$DMG_FILE" | cut -f1)"
        echo ""
        echo "安装方法:"
        echo "1. 双击 $DMG_FILE"
        echo "2. 将Neko云音乐拖到Applications文件夹"
        echo "3. 在应用程序中打开Neko云音乐"
    else
        echo "⚠️ 未找到.dmg文件，请检查构建日志"
    fi
else
    echo "❌ 构建失败，release目录不存在"
fi

cd ..
