export default {
  BASE_URL: 'https://music.cnmsb.xin',
  
  // 音乐相关
  MUSIC_SEARCH: '/api/music/search',
  MUSIC_COVER: (id) => `/api/music/cover/${id}`,
  MUSIC_FILE: (id) => `/api/music/file/${id}`,
  MUSIC_INFO: (id) => `/api/music/info/${id}`,
  MUSIC_LYRICS: (id) => `/api/music/lyrics/${id}`,
  
  // 用户相关
  USER_LOGIN: '/api/user/login',
  USER_REGISTER: '/api/user/register',
  USER_AVATAR: (userId) => `/api/user/avatar/${userId}`,
  USER_AVATAR_UPLOAD: '/api/user/avatar/upload',
  USER_PASSWORD_CHANGE: '/api/user/password/change',
  USER_FAVORITES: '/api/user/favorites',
  USER_FAVORITES_DELETE: (musicId) => `/api/user/favorites/${musicId}`,
  
  // 歌单相关
  PLAYLIST_CREATE: '/api/user/playlist/create',
  PLAYLIST_UPDATE: '/api/user/playlist/update',
  PLAYLIST_DELETE: '/api/user/playlist/delete',
  PLAYLISTS: '/api/user/playlists',
  PLAYLIST_DETAIL: (id) => `/api/playlist/${id}`,
  PLAYLIST_MUSIC: (playlistId) => `/api/user/playlist/music/${playlistId}`,
  PLAYLIST_MUSIC_ADD: '/api/user/playlist/music/add',
  PLAYLIST_MUSIC_REMOVE: '/api/user/playlist/music/remove',
  PLAYLISTS_SEARCH: '/api/playlists/search',
  
  // 搜索相关
  ARTISTS_SEARCH: '/api/artists/search',
  
  // 更新检测
  UPDATE_CHECK: '/version.json'
}