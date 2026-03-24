const fs = require('fs');
const path = require('path');

exports.default = async function(context) {
  console.log('========== 开始极致压缩优化 ==========');
  
  const appOutDir = context.appOutDir;
  let totalSaved = 0;

  // 1. 优化语言文件（只保留 en-US 和 zh-CN）
  console.log('\n[1/12] 优化语言文件...');
  const localesDir = path.join(appOutDir, 'locales');
  if (fs.existsSync(localesDir)) {
    const keepLocales = ['en-US.pak', 'zh-CN.pak'];
    const files = fs.readdirSync(localesDir);
    files.forEach(file => {
      if (!keepLocales.includes(file)) {
        const filePath = path.join(localesDir, file);
        const stats = fs.statSync(filePath);
        totalSaved += stats.size;
        fs.unlinkSync(filePath);
        console.log(`  ✗ 移除: ${file} (${(stats.size / 1024).toFixed(2)} KB)`);
      }
    });
    console.log(`  ✓ 保留: ${keepLocales.join(', ')}`);
  }

  // 2. 移除 Chromium 许可证文件（节省 15 MB）
  console.log('\n[2/12] 移除 Chromium 许可证文件...');
  const licenseFiles = [
    'LICENSES.chromium.html',
    'LICENSE.electron.txt',
    'LICENSE',
    'LICENSES.chromium.os.html'
  ];
  licenseFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 3. 移除崩溃报告处理器（节省 1.5 MB）
  console.log('\n[3/12] 移除崩溃报告处理器...');
  const crashpadFiles = [
    'chrome_crashpad_handler',
    'chrome_crashpad_handler.exe'
  ];
  crashpadFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 4. 移除 SwiftShader（软件渲染器，节省 4.5 MB）
  console.log('\n[4/12] 移除 SwiftShader 软件渲染器...');
  const swiftshaderFiles = [
    'libvk_swiftshader.so',
    'vk_swiftshader.dll',
    'vk_swiftshader_icd.json'
  ];
  swiftshaderFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 5. 移除 Vulkan（节省 2.4 MB）
  console.log('\n[5/12] 移除 Vulkan 支持...');
  const vulkanFiles = [
    'libvulkan.so.1',
    'vulkan-1.dll',
    'libvulkan.so',
    'vulkan-1.dll'
  ];
  vulkanFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 6. 移除 Chrome 资源包（节省 184 KB + 116 KB）
  console.log('\n[6/12] 移除 Chrome 资源包...');
  const chromePakFiles = [
    'chrome_100_percent.pak',
    'chrome_200_percent.pak',
    'resources.pak'
  ];
  chromePakFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024).toFixed(2)} KB)`);
    }
  });

  // 7. 移除 AppArmor 配置（节省 237 字节）
  console.log('\n[7/12] 移除 AppArmor 配置...');
  const apparmorPath = path.join(appOutDir, 'resources', 'apparmor-profile');
  if (fs.existsSync(apparmorPath)) {
    const stats = fs.statSync(apparmorPath);
    totalSaved += stats.size;
    fs.unlinkSync(apparmorPath);
    console.log(`  ✗ 移除: apparmor-profile (${stats.size} B)`);
  }

  // 8. 移除 GPU 相关文件
  console.log('\n[8/12] 移除 GPU 相关文件...');
  const gpuFiles = [
    'vk_swiftshader.dll',
    'd3dcompiler_47.dll',
    'libEGL.dll',
    'libGLESv2.dll',
    'angle_data/vulkan/swiftshader/ vk_swiftshader_icd.json'
  ];
  gpuFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 9. 移除资源和缓存文件
  console.log('\n[9/12] 移除资源和缓存文件...');
  const resourceDirs = [
    'resources/default_app.asar',
    'resources/app.asar.unpacked',
    'resources/default_app.asar.unpacked'
  ];
  resourceDirs.forEach(dir => {
    const dirPath = path.join(appOutDir, dir);
    if (fs.existsSync(dirPath)) {
      const removeDirRecursive = (dirPath) => {
        const files = fs.readdirSync(dirPath);
        files.forEach(file => {
          const filePath = path.join(dirPath, file);
          const stats = fs.statSync(filePath);
          if (stats.isDirectory()) {
            removeDirRecursive(filePath);
          } else {
            totalSaved += stats.size;
            fs.unlinkSync(filePath);
          }
        });
        fs.rmdirSync(dirPath);
      };
      removeDirRecursive(dirPath);
      console.log(`  ✗ 移除: ${dir}`);
    }
  });

  // 10. 移除 ffmpeg（如果不使用媒体功能）
  console.log('\n[10/12] 移除 ffmpeg...');
  const ffmpegFiles = [
    'ffmpeg.dll',
    'libffmpeg.so',
    'ffmpeg'
  ];
  ffmpegFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 11. 移除其他不必要的文件
  console.log('\n[11/12] 移除其他不必要的文件...');
  const otherFiles = [
    'chrome_elf.dll',
    'chrome_child.dll',
    'content_shell.pak',
    'resources.pak',
    'libpdf.so',
    'libwidevinecdm.so',
    'libwidevinecdmadapter.so'
  ];
  otherFiles.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      totalSaved += stats.size;
      fs.unlinkSync(filePath);
      console.log(`  ✗ 移除: ${file} (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    }
  });

  // 12. 移除 V8 快照和快照 blob（节省 700 KB + 332 KB）
  // 注意：V8 快照文件是必需的，不能移除
  console.log('\n[12/12] V8 快照文件（必需，保留）...');
  const v8Files = [
    'v8_context_snapshot.bin',
    'snapshot_blob.bin'
  ];
  v8Files.forEach(file => {
    const filePath = path.join(appOutDir, file);
    if (fs.existsSync(filePath)) {
      const stats = fs.statSync(filePath);
      console.log(`  ✓ 保留: ${file} (${(stats.size / 1024).toFixed(2)} KB)`);
    }
  });

  console.log('\n========== 极致压缩完成 ==========');
  console.log(`总计节省空间: ${(totalSaved / 1024 / 1024).toFixed(2)} MB`);
  console.log('=====================================');
};