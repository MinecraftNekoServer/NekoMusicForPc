const fs = require('fs');
const path = require('path');

exports.default = async function(context) {
  console.log('开始优化打包文件...');
  
  const localesDir = path.join(context.appOutDir, 'locales');
  
  // 检查 locales 目录是否存在
  if (!fs.existsSync(localesDir)) {
    console.log('未找到 locales 目录，跳过清理');
    return;
  }
  
  // 需要保留的语言文件
  const keepLocales = ['en-US.pak', 'zh-CN.pak'];
  
  // 读取所有语言文件
  const files = fs.readdirSync(localesDir);
  
  let removedCount = 0;
  let totalSize = 0;
  
  // 删除不需要的语言文件
  files.forEach(file => {
    if (!keepLocales.includes(file)) {
      const filePath = path.join(localesDir, file);
      const stats = fs.statSync(filePath);
      totalSize += stats.size;
      fs.unlinkSync(filePath);
      removedCount++;
      console.log(`已移除: ${file} (${(stats.size / 1024).toFixed(2)} KB)`);
    }
  });
  
  console.log(`优化完成！已移除 ${removedCount} 个语言文件，节省 ${(totalSize / 1024 / 1024).toFixed(2)} MB 空间`);
  console.log(`保留的语言: ${keepLocales.join(', ')}`);
};