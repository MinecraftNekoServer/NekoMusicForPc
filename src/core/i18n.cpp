/**
 * @file i18n.cpp
 * @brief 简易国际化实现
 */

#include "i18n.h"
#include <QMap>

I18n &I18n::instance()
{
    static I18n i;
    return i;
}

void I18n::setLanguage(Language lang)
{
    m_lang = lang;
}

static QMap<QString, QString> zhCN()
{
    QMap<QString, QString> m;
    // --- 基础通用操作 ---
    m["search"] = QStringLiteral("搜索");
    m["searchPlaceholder"] = QStringLiteral("搜索音乐...");
    m["loading"] = QStringLiteral("加载中...");
    m["noData"] = QStringLiteral("暂无数据");
    m["cancel"] = QStringLiteral("取消");
    m["confirm"] = QStringLiteral("确认");
    m["delete"] = QStringLiteral("删除");
    m["edit"] = QStringLiteral("编辑");
    m["save"] = QStringLiteral("保存");
    m["close"] = QStringLiteral("关闭");
    m["submit"] = QStringLiteral("提交");
    m["back"] = QStringLiteral("返回");
    m["refresh"] = QStringLiteral("刷新");
    m["clear"] = QStringLiteral("清空");
    m["all"] = QStringLiteral("全部");
    m["create"] = QStringLiteral("创建");
    m["rename"] = QStringLiteral("重命名");
    m["actions"] = QStringLiteral("操作");
    m["none"] = QStringLiteral("无");
    m["unknown"] = QStringLiteral("未知");
    m["inputRequired"] = QStringLiteral("请输入");
    m["inputValid"] = QStringLiteral("请输入有效的内容");
    m["justNow"] = QStringLiteral("刚刚");
    m["minutesAgo"] = QStringLiteral("分钟前");
    m["hoursAgo"] = QStringLiteral("小时前");
    m["daysAgo"] = QStringLiteral("天前");
    m["monthsAgo"] = QStringLiteral("个月前");
    m["yearsAgo"] = QStringLiteral("年前");

    // --- 播放器 & 媒体控制 ---
    m["play"] = QStringLiteral("播放");
    m["pause"] = QStringLiteral("暂停");
    m["previous"] = QStringLiteral("上一首");
    m["next"] = QStringLiteral("下一首");
    m["favorite"] = QStringLiteral("收藏");
    m["unfavorite"] = QStringLiteral("取消收藏");
    m["playAll"] = QStringLiteral("播放全部");
    m["playlist"] = QStringLiteral("播放列表");
    m["emptyPlaylist"] = QStringLiteral("播放列表为空");
    m["notPlaying"] = QStringLiteral("未播放");
    m["volume"] = QStringLiteral("音量");
    m["muted"] = QStringLiteral("已静音");
    m["unmuted"] = QStringLiteral("取消静音");
    m["toggleMute"] = QStringLiteral("取消静音");
    m["mute"] = QStringLiteral("静音");
    m["desktopLyrics"] = QStringLiteral("词");
    m["desktopLyricsEnabled"] = QStringLiteral("桌面歌词已开启");
    m["desktopLyricsDisabled"] = QStringLiteral("桌面歌词已关闭");
    m["playModeList"] = QStringLiteral("列表循环");
    m["playModeSingle"] = QStringLiteral("单曲循环");
    m["playModeRandom"] = QStringLiteral("随机播放");
    m["playModeOff"] = QStringLiteral("不循环");
    m["minimize"] = QStringLiteral("最小化");
    m["maximize"] = QStringLiteral("全屏");
    m["closeWindow"] = QStringLiteral("关闭窗口");
    m["duration"] = QStringLiteral("时长");
    m["title"] = QStringLiteral("标题");
    m["cover"] = QStringLiteral("封面");
    m["songs"] = QStringLiteral("歌曲");
    m["playlists"] = QStringLiteral("歌单");
    m["album"] = QStringLiteral("专辑");
    m["artist"] = QStringLiteral("艺术家");
    m["count"] = QStringLiteral("数量");
    m["musicCount"] = QStringLiteral("首歌曲");
    m["songList"] = QStringLiteral("歌曲列表");

    // --- 导航 & 视图名称 ---
    m["home"] = QStringLiteral("首页");
    m["myPlaylists"] = QStringLiteral("我的歌单");
    m["myPlaylistsTitle"] = QStringLiteral("我的歌单");
    m["favorites"] = QStringLiteral("我喜欢的");
    m["myFavorites"] = QStringLiteral("我喜欢的音乐");
    m["favoritePlaylists"] = QStringLiteral("收藏的歌单");
    m["favoritePlaylistsTitle"] = QStringLiteral("收藏的歌单");
    m["recentPlay"] = QStringLiteral("最近播放");
    m["ranking"] = QStringLiteral("排行榜");
    m["latest"] = QStringLiteral("最新音乐");
    m["library"] = QStringLiteral("音乐库");
    m["topSongs"] = QStringLiteral("热门歌曲");
    m["hotMusic"] = QStringLiteral("热门音乐");
    m["hotMusicDesc"] = QStringLiteral("播放次数最高的热门歌曲");
    m["latestMusic"] = QStringLiteral("最新音乐");
    m["latestMusicDesc"] = QStringLiteral("刚刚上传的最新歌曲");
    m["newSongs"] = QStringLiteral("新歌上架");
    m["discoverMusic"] = QStringLiteral("去发现音乐");
    m["recommendPlaylists"] = QStringLiteral("推荐歌单");
    m["recommend_playlists"] = QStringLiteral("推荐歌单");
    m["hotMusic"] = QStringLiteral("热门音乐");
    m["hot_music"] = QStringLiteral("热门音乐");
    m["latestMusic"] = QStringLiteral("最新音乐");
    m["latest_music"] = QStringLiteral("最新音乐");
    m["searchMusic"] = QStringLiteral("单曲");
    m["searchPlaylist"] = QStringLiteral("歌单");
    m["searchResults"] = QStringLiteral("搜索结果");
    m["noResults"] = QStringLiteral("暂无搜索结果");

    // --- 账户 & 登录注册 ---
    m["login"] = QStringLiteral("登录");
    m["logout"] = QStringLiteral("退出登录");
    m["register"] = QStringLiteral("注册");
    m["email"] = QStringLiteral("邮箱");
    m["password"] = QStringLiteral("密码");
    m["username"] = QStringLiteral("用户名");
    m["nickname"] = QStringLiteral("昵称");
    m["emailOrName"] = QStringLiteral("邮箱/名称");
    m["verificationCode"] = QStringLiteral("验证码");
    m["verifyCode"] = QStringLiteral("验证码");
    m["getCode"] = QStringLiteral("获取验证码");
    m["sendCode"] = QStringLiteral("发送验证码");
    m["resendCode"] = QStringLiteral("重新发送");
    m["registerTime"] = QStringLiteral("注册时间");
    m["accountInfo"] = QStringLiteral("账号信息");
    m["accountSettings"] = QStringLiteral("账号设置");
    m["notLoggedIn"] = QStringLiteral("未登录");
    m["goToLogin"] = QStringLiteral("去登录");
    m["welcome"] = QStringLiteral("欢迎回来");
    m["welcomeSubtitle"] = QStringLiteral("发现好音乐，享受每一刻");
    m["goodMorning"] = QStringLiteral("早上好");
    m["goodAfternoon"] = QStringLiteral("下午好");
    m["goodEvening"] = QStringLiteral("晚上好");
    m["lateNight"] = QStringLiteral("夜深了");

    // --- 歌单特定操作 ---
    m["createPlaylist"] = QStringLiteral("创建歌单");
    m["createFirstPlaylist"] = QStringLiteral("创建第一个歌单");
    m["noPlaylists"] = QStringLiteral("暂无歌单，请先创建歌单");
    m["playlistName"] = QStringLiteral("歌单名称");
    m["playlistDescription"] = QStringLiteral("歌单描述");
    m["playlistNamePlaceholder"] = QStringLiteral("输入歌单名称");
    m["playlistDescPlaceholder"] = QStringLiteral("输入歌单描述");
    m["creator"] = QStringLiteral("创建者");
    m["editPlaylist"] = QStringLiteral("编辑歌单");
    m["deletePlaylist"] = QStringLiteral("删除歌单");
    m["renamePlaylist"] = QStringLiteral("重命名歌单");
    m["modifyPlaylistDesc"] = QStringLiteral("修改歌单描述");
    m["inputNewPlaylistName"] = QStringLiteral("输入新歌单名称");
    m["inputPlaylistDesc"] = QStringLiteral("输入歌单描述");
    m["inputPlaylistName"] = QStringLiteral("请输入歌单名称");
    m["collectPlaylist"] = QStringLiteral("收藏歌单");
    m["collectedPlaylist"] = QStringLiteral("已收藏歌单");
    m["uncollectPlaylist"] = QStringLiteral("取消收藏");
    m["removeFromPlaylist"] = QStringLiteral("从歌单中移除");
    m["addToPlaylist"] = QStringLiteral("添加到歌单");
    m["addToPlaylistTitle"] = QStringLiteral("添加到歌单");
    m["selectPlaylist"] = QStringLiteral("选择歌单");
    m["orCreateNewPlaylist"] = QStringLiteral("或创建新歌单");
    m["addMusicToPlaylist"] = QStringLiteral("添加音乐到歌单");

    // --- 系统设置 & 缓存 & 更新 ---
    m["settings"] = QStringLiteral("设置");
    m["playSettings"] = QStringLiteral("播放设置");
    m["language"] = QStringLiteral("语言");
    m["languageSettings"] = QStringLiteral("语言设置");
    m["languageNya"] = QStringLiteral("喵语中文");
    m["languageChinese"] = QStringLiteral("简体中文");
    m["languageEnglish"] = QStringLiteral("English");
    m["musicCache"] = QStringLiteral("音乐缓存");
    m["musicCacheDesc"] = QStringLiteral("自动缓存听过的音乐到本地下载目录");
    m["cachePath"] = QStringLiteral("缓存路径");
    m["cacheEnabled"] = QStringLiteral("音乐缓存已开启");
    m["cacheDisabled"] = QStringLiteral("音乐缓存已关闭");
    m["loadFromCache"] = QStringLiteral("从缓存加载");
    m["about"] = QStringLiteral("关于");
    m["appVersion"] = QStringLiteral("版本号");
    m["currentVersion"] = QStringLiteral("当前版本");
    m["githubRepo"] = QStringLiteral("GitHub 仓库");
    m["apiDocs"] = QStringLiteral("API 文档");
    m["createdAt"] = QStringLiteral("创建时间");
    m["updatedAt"] = QStringLiteral("更新时间");
    m["description"] = QStringLiteral("描述");
    m["checkUpdate"] = QStringLiteral("检查更新");
    m["checking"] = QStringLiteral("检查中...");
    m["updateAvailable"] = QStringLiteral("发现新版本");
    m["newVersionAvailable"] = QStringLiteral("发现新版本");
    m["newVersion"] = QStringLiteral("新版本");
    m["remindLater"] = QStringLiteral("稍后提醒");
    m["updateNow"] = QStringLiteral("立即更新");
    m["downloading"] = QStringLiteral("下载中...");
    m["downloadUpdate"] = QStringLiteral("立即下载");
    m["downloadComplete"] = QStringLiteral("下载完成");
    m["downloadFailed"] = QStringLiteral("下载失败");
    m["downloadCancelled"] = QStringLiteral("下载已取消");
    m["installNow"] = QStringLiteral("立即安装");
    m["installLater"] = QStringLiteral("稍后安装");
    m["isLatest"] = QStringLiteral("当前已是最新版本");
    m["versionDownloaded"] = QStringLiteral("版本 {version} 已下载完成");
    m["updateDownloadComplete"] = QStringLiteral("更新下载完成");

    // --- 交互反馈 (成功/失败/校验) ---
    m["operationFailed"] = QStringLiteral("操作失败");
    m["networkError"] = QStringLiteral("网络错误，请检查服务器连接");
    m["networkErrorRetry"] = QStringLiteral("网络错误，请重试");
    m["loginSuccess"] = QStringLiteral("登录成功，欢迎回来！");
    m["loginFailed"] = QStringLiteral("登录失败");
    m["logoutSuccess"] = QStringLiteral("已退出登录");
    m["registerSuccess"] = QStringLiteral("注册成功，请登录");
    m["registerFailed"] = QStringLiteral("注册失败");
    m["createSuccess"] = QStringLiteral("创建成功");
    m["createFailed"] = QStringLiteral("创建失败");
    m["createPlaylistSuccess"] = QStringLiteral("歌单创建成功");
    m["createPlaylistFailed"] = QStringLiteral("创建失败");
    m["deleteSuccess"] = QStringLiteral("删除成功");
    m["deleteFailed"] = QStringLiteral("删除失败");
    m["modifySuccess"] = QStringLiteral("修改成功");
    m["renameSuccess"] = QStringLiteral("重命名成功");
    m["updateSuccess"] = QStringLiteral("更新成功");
    m["favoriteSuccess"] = QStringLiteral("收藏成功");
    m["favoriteFailed"] = QStringLiteral("收藏失败");
    m["cancelFavoriteSuccess"] = QStringLiteral("已取消收藏");
    m["cancelFavoriteFailed"] = QStringLiteral("取消收藏失败");
    m["cancelFavoritePlaylistSuccess"] = QStringLiteral("已取消收藏歌单");
    m["cancelFavoritePlaylistFailed"] = QStringLiteral("取消收藏失败");
    m["collectPlaylistSuccess"] = QStringLiteral("已收藏歌单");
    m["collectPlaylistFailed"] = QStringLiteral("收藏失败");
    m["unfavoriteSuccess"] = QStringLiteral("取消收藏成功");
    m["addToPlaylistSuccess"] = QStringLiteral("添加到歌单成功");
    m["addToPlaylistFailed"] = QStringLiteral("添加失败");
    m["addSuccess"] = QStringLiteral("添加成功");
    m["removeSuccess"] = QStringLiteral("移除成功");
    m["removeFailed"] = QStringLiteral("移除失败");
    m["musicRemovedSuccess"] = QStringLiteral("音乐已从歌单中移除");
    m["switchLanguageSuccess"] = QStringLiteral("语言设置已保存");
    m["switchLanguageFailed"] = QStringLiteral("切换语言失败");
    m["checkUpdateFailed"] = QStringLiteral("检查更新失败，请稍后重试");
    m["sendVerifyCodeFailed"] = QStringLiteral("发送验证码失败");
    m["verifyCodeSent"] = QStringLiteral("验证码已发送至您的邮箱");
    m["codeSent"] = QStringLiteral("验证码已发送");
    m["inputEmail"] = QStringLiteral("请先输入邮箱地址");
    m["inputValidEmail"] = QStringLiteral("请输入有效的邮箱地址");
    m["emailRequired"] = QStringLiteral("请输入邮箱");
    m["passwordRequired"] = QStringLiteral("请输入密码");
    m["codeRequired"] = QStringLiteral("请输入验证码");
    m["nameRequired"] = QStringLiteral("请输入昵称");
    m["inputNameRequired"] = QStringLiteral("请输入歌单名称");
    m["fillUsernameAndPassword"] = QStringLiteral("请填写用户名和密码");
    m["fillEmail"] = QStringLiteral("请填写邮箱");
    m["fillVerificationCode"] = QStringLiteral("请填写验证码");
    m["pleaseLoginFirst"] = QStringLiteral("请先登录");
    m["deleteConfirm"] = QStringLiteral("确定要删除吗？");
    m["confirmRemove"] = QStringLiteral("确认移除");
    m["confirmRemoveMessage"] = QStringLiteral("确定要从歌单中移除这首音乐吗？");
    m["deleteConfirmWithPlaylistName"] = QStringLiteral("确定要删除歌单\"{name}\"吗？此操作不可恢复。");
    m["invalidEmail"] = QStringLiteral("邮箱格式不正确");
    m["emailPlaceholder"] = QStringLiteral("请输入邮箱");
    m["passwordPlaceholder"] = QStringLiteral("请输入密码");
    m["namePlaceholder"] = QStringLiteral("请输入名称（昵称）");
    m["inputKeywordToSearch"] = QStringLiteral("输入关键词开始搜索");

    // --- 异常与调试 ---
    m["emptyMusic"] = QStringLiteral("音乐数据为空");
    m["musicInPlaylist"] = QStringLiteral("音乐已在播放列表中");
    m["invalidMusicList"] = QStringLiteral("无效的音乐列表");
    m["musicExistsSkip"] = QStringLiteral("音乐已存在，跳过");
    m["downloadMusicFailedSkipCache"] = QStringLiteral("下载音乐失败，跳过缓存");
    m["getFilePathFailedSkipCache"] = QStringLiteral("获取文件路径失败，跳过缓存");
    m["writeToFileFailedSkipCache"] = QStringLiteral("写入文件失败，跳过缓存");
    m["parseRecentPlayListFailed"] = QStringLiteral("解析最近播放列表失败");
    m["parseMusicFailed"] = QStringLiteral("解析音乐失败");

    return m;
}

static QMap<QString, QString> nyaCN()
{
    QMap<QString, QString> m = zhCN();
    // 喵语风格：在句末加"喵~""呐""的说"等
    m["search"] = QStringLiteral("搜索喵~");
    m["searchPlaceholder"] = QStringLiteral("搜索音乐喵~...");
    m["loading"] = QStringLiteral("加载中喵~...");
    m["noData"] = QStringLiteral("暂无数据喵~");
    m["cancel"] = QStringLiteral("取消喵~");
    m["confirm"] = QStringLiteral("确认喵~");
    m["delete"] = QStringLiteral("删除喵~");
    m["edit"] = QStringLiteral("编辑喵~");
    m["save"] = QStringLiteral("保存喵~");
    m["close"] = QStringLiteral("关闭喵~");
    m["submit"] = QStringLiteral("提交喵~");
    m["back"] = QStringLiteral("返回喵~");
    m["refresh"] = QStringLiteral("刷新喵~");
    m["clear"] = QStringLiteral("清空喵~");
    m["all"] = QStringLiteral("全部喵~");
    m["create"] = QStringLiteral("创建喵~");
    m["rename"] = QStringLiteral("重命名喵~");
    m["play"] = QStringLiteral("播放喵~");
    m["pause"] = QStringLiteral("暂停喵~");
    m["previous"] = QStringLiteral("上一首喵~");
    m["next"] = QStringLiteral("下一首喵~");
    m["favorite"] = QStringLiteral("收藏喵~");
    m["unfavorite"] = QStringLiteral("取消收藏喵~");
    m["playAll"] = QStringLiteral("播放全部喵~");
    m["playlist"] = QStringLiteral("播放列表喵~");
    m["emptyPlaylist"] = QStringLiteral("播放列表为空喵~");
    m["notPlaying"] = QStringLiteral("未在播放喵~");
    m["volume"] = QStringLiteral("音量喵~");
    m["muted"] = QStringLiteral("已静音喵~");
    m["songs"] = QStringLiteral("歌曲喵~");
    m["playlists"] = QStringLiteral("歌单喵~");
    m["album"] = QStringLiteral("专辑喵~");
    m["artist"] = QStringLiteral("艺术家喵~");
    m["home"] = QStringLiteral("首页喵~");
    m["myPlaylists"] = QStringLiteral("我的歌单喵~");
    m["myPlaylistsTitle"] = QStringLiteral("我的歌单喵~");
    m["favorites"] = QStringLiteral("我喜欢的喵~");
    m["myFavorites"] = QStringLiteral("我喜欢的音乐喵~");
    m["favoritePlaylists"] = QStringLiteral("收藏的歌单喵~");
    m["favoritePlaylistsTitle"] = QStringLiteral("收藏的歌单喵~");
    m["recentPlay"] = QStringLiteral("最近播放喵~");
    m["ranking"] = QStringLiteral("排行榜喵~");
    m["latest"] = QStringLiteral("最新音乐喵~");
    m["library"] = QStringLiteral("音乐库喵~");
    m["topSongs"] = QStringLiteral("热门歌曲喵~");
    m["hotMusic"] = QStringLiteral("热门音乐喵~");
    m["hotMusicDesc"] = QStringLiteral("播放次数最高的热门歌曲喵~");
    m["latestMusic"] = QStringLiteral("最新音乐喵~");
    m["latestMusicDesc"] = QStringLiteral("刚刚上传的最新歌曲喵~");
    m["newSongs"] = QStringLiteral("新歌上架喵~");
    m["discoverMusic"] = QStringLiteral("去发现音乐喵~");
    m["recommendPlaylists"] = QStringLiteral("推荐歌单喵~");
    m["recommend_playlists"] = QStringLiteral("推荐歌单喵~");
    m["hotMusic"] = QStringLiteral("热门音乐喵~");
    m["hot_music"] = QStringLiteral("热门音乐喵~");
    m["latestMusic"] = QStringLiteral("最新音乐喵~");
    m["latest_music"] = QStringLiteral("最新音乐喵~");
    m["login"] = QStringLiteral("登录喵~");
    m["logout"] = QStringLiteral("退出登录喵~");
    m["register"] = QStringLiteral("注册喵~");
    m["email"] = QStringLiteral("邮箱喵~");
    m["password"] = QStringLiteral("密码喵~");
    m["settings"] = QStringLiteral("设置喵~");
    m["playSettings"] = QStringLiteral("播放设置喵~");
    m["language"] = QStringLiteral("语言喵~");
    m["languageSettings"] = QStringLiteral("语言设置喵~");
    m["languageNya"] = QStringLiteral("喵语中文喵~");
    m["languageChinese"] = QStringLiteral("简体中文");
    m["languageEnglish"] = QStringLiteral("English");
    m["musicCache"] = QStringLiteral("音乐缓存喵~");
    m["musicCacheDesc"] = QStringLiteral("自动缓存听过的音乐到本地下载目录喵~");
    m["cachePath"] = QStringLiteral("缓存路径喵~");
    m["about"] = QStringLiteral("关于喵~");
    m["appVersion"] = QStringLiteral("版本号喵~");
    m["currentVersion"] = QStringLiteral("当前版本喵~");
    m["githubRepo"] = QStringLiteral("GitHub 仓库喵~");
    m["apiDocs"] = QStringLiteral("API 文档喵~");
    m["checkUpdate"] = QStringLiteral("检查更新喵~");
    m["checking"] = QStringLiteral("检查中喵~...");
    m["isLatest"] = QStringLiteral("当前已是最新版本喵~");
    m["createPlaylist"] = QStringLiteral("创建歌单喵~");
    m["playlistName"] = QStringLiteral("歌单名称喵~");
    m["playlistDescription"] = QStringLiteral("歌单描述喵~");
    m["musicCount"] = QStringLiteral("首歌曲喵~");
    m["goToLogin"] = QStringLiteral("去登录喵~");
    m["unknown"] = QStringLiteral("未知喵~");
    m["playModeList"] = QStringLiteral("列表循环喵~");
    m["playModeSingle"] = QStringLiteral("单曲循环喵~");
    m["playModeRandom"] = QStringLiteral("随机播放喵~");
    m["playModeOff"] = QStringLiteral("不循环喵~");
    return m;
}

static QMap<QString, QString> enUS()
{
    QMap<QString, QString> m;
    // --- General Operations ---
    m["search"] = "Search";
    m["searchPlaceholder"] = "Search music...";
    m["loading"] = "Loading...";
    m["noData"] = "No Data";
    m["cancel"] = "Cancel";
    m["confirm"] = "Confirm";
    m["delete"] = "Delete";
    m["edit"] = "Edit";
    m["save"] = "Save";
    m["close"] = "Close";
    m["submit"] = "Submit";
    m["back"] = "Back";
    m["refresh"] = "Refresh";
    m["clear"] = "Clear";
    m["all"] = "All";
    m["create"] = "Create";
    m["rename"] = "Rename";
    m["actions"] = "Actions";
    m["none"] = "None";
    m["unknown"] = "Unknown";
    m["inputRequired"] = "Please input";
    m["inputValid"] = "Please enter valid content";
    m["justNow"] = "Just now";
    m["minutesAgo"] = "m ago";
    m["hoursAgo"] = "h ago";
    m["daysAgo"] = "d ago";
    m["monthsAgo"] = "mo ago";
    m["yearsAgo"] = "y ago";

    // --- Player & Media Controls ---
    m["play"] = "Play";
    m["pause"] = "Pause";
    m["previous"] = "Previous";
    m["next"] = "Next";
    m["favorite"] = "Favorite";
    m["unfavorite"] = "Unfavorite";
    m["playAll"] = "Play All";
    m["playlist"] = "Playlist";
    m["emptyPlaylist"] = "Playlist is empty";
    m["notPlaying"] = "Not Playing";
    m["volume"] = "Volume";
    m["muted"] = "Muted";
    m["unmuted"] = "Unmuted";
    m["toggleMute"] = "Toggle Mute";
    m["mute"] = "Mute";
    m["desktopLyrics"] = "Lyrics";
    m["desktopLyricsEnabled"] = "Desktop lyrics enabled";
    m["desktopLyricsDisabled"] = "Desktop lyrics disabled";
    m["playModeList"] = "List Loop";
    m["playModeSingle"] = "Single Loop";
    m["playModeRandom"] = "Random";
    m["playModeOff"] = "No Loop";
    m["minimize"] = "Minimize";
    m["maximize"] = "Maximize";
    m["closeWindow"] = "Close Window";
    m["duration"] = "Duration";
    m["title"] = "Title";
    m["cover"] = "Cover";
    m["songs"] = "Songs";
    m["playlists"] = "Playlists";
    m["album"] = "Album";
    m["artist"] = "Artist";
    m["count"] = "Count";
    m["musicCount"] = "Songs";
    m["songList"] = "Song List";

    // --- Navigation & Views ---
    m["home"] = "Home";
    m["myPlaylists"] = "My Playlists";
    m["myPlaylistsTitle"] = "My Playlists";
    m["favorites"] = "My Favorites";
    m["myFavorites"] = "My Favorite Music";
    m["favoritePlaylists"] = "Favorite Playlists";
    m["favoritePlaylistsTitle"] = "Favorite Playlists";
    m["recentPlay"] = "Recently Played";
    m["ranking"] = "Ranking";
    m["latest"] = "Latest";
    m["library"] = "Library";
    m["topSongs"] = "Top Songs";
    m["hotMusic"] = "Hot Music";
    m["hotMusicDesc"] = "Most played popular songs";
    m["latestMusic"] = "Latest Music";
    m["latestMusicDesc"] = "Recently uploaded songs";
    m["newSongs"] = "New Songs";
    m["discoverMusic"] = "Discover Music";
    m["recommendPlaylists"] = "Recommended Playlists";
    m["recommend_playlists"] = "Recommended Playlists";
    m["hotMusic"] = "Hot Music";
    m["hot_music"] = "Hot Music";
    m["latestMusic"] = "Latest Music";
    m["latest_music"] = "Latest Music";
    m["searchMusic"] = "Songs";
    m["searchPlaylist"] = "Playlists";
    m["searchResults"] = "Search Results";
    m["noResults"] = "No results found";

    // --- Account & Auth ---
    m["login"] = "Login";
    m["logout"] = "Logout";
    m["register"] = "Register";
    m["email"] = "Email";
    m["password"] = "Password";
    m["username"] = "Username";
    m["nickname"] = "Nickname";
    m["emailOrName"] = "Email/Name";
    m["verificationCode"] = "Verification Code";
    m["verifyCode"] = "Verify Code";
    m["getCode"] = "Get Code";
    m["sendCode"] = "Send Code";
    m["resendCode"] = "Resend";
    m["registerTime"] = "Registration Time";
    m["accountInfo"] = "Account Info";
    m["accountSettings"] = "Account Settings";
    m["notLoggedIn"] = "Not Logged In";
    m["goToLogin"] = "Go to Login";
    m["welcome"] = "Welcome Back";
    m["welcomeSubtitle"] = "Discover great music, enjoy every moment";
    m["goodMorning"] = "Good Morning";
    m["goodAfternoon"] = "Good Afternoon";
    m["goodEvening"] = "Good Evening";
    m["lateNight"] = "Late Night";

    // --- Playlist Operations ---
    m["createPlaylist"] = "Create Playlist";
    m["createFirstPlaylist"] = "Create your first playlist";
    m["noPlaylists"] = "No playlists yet, please create one";
    m["playlistName"] = "Playlist Name";
    m["playlistDescription"] = "Description";
    m["playlistNamePlaceholder"] = "Enter playlist name";
    m["playlistDescPlaceholder"] = "Enter playlist description";
    m["creator"] = "Creator";
    m["editPlaylist"] = "Edit Playlist";
    m["deletePlaylist"] = "Delete Playlist";
    m["renamePlaylist"] = "Rename Playlist";
    m["modifyPlaylistDesc"] = "Edit Description";
    m["inputNewPlaylistName"] = "Enter new playlist name";
    m["inputPlaylistDesc"] = "Enter description";
    m["inputPlaylistName"] = "Please enter playlist name";
    m["collectPlaylist"] = "Collect Playlist";
    m["collectedPlaylist"] = "Collected";
    m["uncollectPlaylist"] = "Uncollect";
    m["removeFromPlaylist"] = "Remove from Playlist";
    m["addToPlaylist"] = "Add to Playlist";
    m["addToPlaylistTitle"] = "Add to Playlist";
    m["selectPlaylist"] = "Select Playlist";
    m["orCreateNewPlaylist"] = "Or create new playlist";
    m["addMusicToPlaylist"] = "Add music to playlist";

    // --- Settings & Cache & Update ---
    m["settings"] = "Settings";
    m["playSettings"] = "Playback Settings";
    m["language"] = "Language";
    m["languageSettings"] = "Language Settings";
    m["languageNya"] = "Nya Chinese";
    m["languageChinese"] = "简体中文";
    m["languageEnglish"] = "English";
    m["musicCache"] = "Music Cache";
    m["musicCacheDesc"] = "Automatically cache played music to local directory";
    m["cachePath"] = "Cache Path";
    m["cacheEnabled"] = "Cache Enabled";
    m["cacheDisabled"] = "Cache Disabled";
    m["loadFromCache"] = "Load from Cache";
    m["about"] = "About";
    m["appVersion"] = "Version";
    m["currentVersion"] = "Current Version";
    m["githubRepo"] = "GitHub Repository";
    m["apiDocs"] = "API Docs";
    m["createdAt"] = "Created At";
    m["updatedAt"] = "Updated At";
    m["description"] = "Description";
    m["checkUpdate"] = "Check for Updates";
    m["checking"] = "Checking...";
    m["updateAvailable"] = "New version available";
    m["newVersionAvailable"] = "New version available";
    m["newVersion"] = "New Version";
    m["remindLater"] = "Remind Later";
    m["updateNow"] = "Update Now";
    m["downloading"] = "Downloading...";
    m["downloadUpdate"] = "Download Now";
    m["downloadComplete"] = "Download Complete";
    m["downloadFailed"] = "Download Failed";
    m["downloadCancelled"] = "Download Cancelled";
    m["installNow"] = "Install Now";
    m["installLater"] = "Install Later";
    m["isLatest"] = "Already the latest version";
    m["versionDownloaded"] = "Version {version} downloaded";
    m["updateDownloadComplete"] = "Update download complete";

    // --- Feedback ---
    m["operationFailed"] = "Operation Failed";
    m["networkError"] = "Network Error, please check server connection";
    m["networkErrorRetry"] = "Network error, please try again";
    m["loginSuccess"] = "Login Successful, welcome back!";
    m["loginFailed"] = "Login Failed";
    m["logoutSuccess"] = "Logged out successfully";
    m["registerSuccess"] = "Registered successfully, please login";
    m["registerFailed"] = "Registration Failed";
    m["createSuccess"] = "Created Successfully";
    m["createFailed"] = "Creation Failed";
    m["createPlaylistSuccess"] = "Playlist created successfully";
    m["createPlaylistFailed"] = "Failed to create playlist";
    m["deleteSuccess"] = "Deleted successfully";
    m["deleteFailed"] = "Deletion failed";
    m["modifySuccess"] = "Modified successfully";
    m["renameSuccess"] = "Renamed successfully";
    m["updateSuccess"] = "Updated successfully";
    m["favoriteSuccess"] = "Added to favorites";
    m["favoriteFailed"] = "Failed to favorite";
    m["cancelFavoriteSuccess"] = "Removed from favorites";
    m["cancelFavoriteFailed"] = "Failed to remove favorite";
    m["cancelFavoritePlaylistSuccess"] = "Uncollected playlist";
    m["cancelFavoritePlaylistFailed"] = "Failed to uncollect";
    m["collectPlaylistSuccess"] = "Collected playlist";
    m["collectPlaylistFailed"] = "Failed to collect";
    m["unfavoriteSuccess"] = "Unfavorited successfully";
    m["addToPlaylistSuccess"] = "Added to playlist successfully";
    m["addToPlaylistFailed"] = "Failed to add";
    m["addSuccess"] = "Added successfully";
    m["removeSuccess"] = "Removed successfully";
    m["removeFailed"] = "Removal failed";
    m["musicRemovedSuccess"] = "Music removed from playlist";
    m["switchLanguageSuccess"] = "Language settings saved";
    m["switchLanguageFailed"] = "Failed to switch language";
    m["checkUpdateFailed"] = "Failed to check for updates";
    m["sendVerifyCodeFailed"] = "Failed to send code";
    m["verifyCodeSent"] = "Verification code sent to your email";
    m["codeSent"] = "Code sent";
    m["inputEmail"] = "Please enter email address";
    m["inputValidEmail"] = "Please enter a valid email";
    m["emailRequired"] = "Email is required";
    m["passwordRequired"] = "Password is required";
    m["codeRequired"] = "Verification code is required";
    m["nameRequired"] = "Nickname is required";
    m["inputNameRequired"] = "Playlist name is required";
    m["fillUsernameAndPassword"] = "Please fill username and password";
    m["fillEmail"] = "Please fill email";
    m["fillVerificationCode"] = "Please fill verification code";
    m["pleaseLoginFirst"] = "Please login first";
    m["deleteConfirm"] = "Are you sure you want to delete?";
    m["confirmRemove"] = "Confirm Removal";
    m["confirmRemoveMessage"] = "Are you sure you want to remove this music from the playlist?";
    m["deleteConfirmWithPlaylistName"] = "Are you sure you want to delete playlist \"{name}\"? This cannot be undone.";
    m["invalidEmail"] = "Invalid email format";
    m["emailPlaceholder"] = "Enter email";
    m["passwordPlaceholder"] = "Enter password";
    m["namePlaceholder"] = "Enter nickname";
    m["inputKeywordToSearch"] = "Enter keyword to search";

    // --- Errors & Debug ---
    m["emptyMusic"] = "Music data is empty";
    m["musicInPlaylist"] = "Music is already in playlist";
    m["invalidMusicList"] = "Invalid music list";
    m["musicExistsSkip"] = "Music already exists, skipping";
    m["downloadMusicFailedSkipCache"] = "Failed to download, skipping cache";
    m["getFilePathFailedSkipCache"] = "Failed to get path, skipping cache";
    m["writeToFileFailedSkipCache"] = "Failed to write file, skipping cache";
    m["parseRecentPlayListFailed"] = "Failed to parse recent plays";
    m["parseMusicFailed"] = "Failed to parse music";

    return m;
}

QString I18n::tr(const QString &key) const
{
    const auto &map = (m_lang == EnUS) ? enUS() : (m_lang == NyaCN) ? nyaCN() : zhCN();
    auto it = map.constFind(key);
    if (it != map.constEnd())
        return it.value();
    return key;
}

QString I18n::settingsTitle() const { return tr("settings"); }
QString I18n::version() const { return tr("appVersion"); }
QString I18n::system() const { return tr("system"); }
QString I18n::languageLabel() const { return tr("language"); }
QString I18n::languageNya() const { return tr("languageNya"); }
QString I18n::languageChinese() const { return tr("languageChinese"); }
QString I18n::languageEnglish() const { return tr("languageEnglish"); }
QString I18n::languageZhCN() const { return tr("languageChinese"); }
QString I18n::languageEnUS() const { return tr("languageEnglish"); }
