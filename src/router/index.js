import { createRouter, createWebHashHistory } from 'vue-router'

const routes = [
  {
    path: '/',
    redirect: '/home'
  },
  {
    path: '/home',
    name: 'Home',
    component: () => import('../views/HomeView.vue')
  },
  {
    path: '/ranking',
    name: 'Ranking',
    component: () => import('../views/RankingView.vue')
  },
  {
    path: '/search',
    name: 'Search',
    component: () => import('../views/SearchView.vue')
  },
  {
    path: '/favorites',
    name: 'Favorites',
    component: () => import('../views/FavoritesView.vue')
  },
  {
    path: '/favorite-playlists',
    name: 'FavoritePlaylists',
    component: () => import('../views/FavoritePlaylistsView.vue')
  },
  {
    path: '/recent',
    name: 'RecentPlay',
    component: () => import('../views/RecentPlayView.vue')
  },
  {
    path: '/player',
    name: 'Player',
    component: () => import('../views/PlayerView.vue')
  },
  {
    path: '/playlists',
    name: 'Playlists',
    component: () => import('../views/PlaylistsView.vue')
  },
  {
    path: '/playlist/:id',
    name: 'PlaylistDetail',
    component: () => import('../views/PlaylistDetailView.vue')
  },
  {
    path: '/settings',
    name: 'Settings',
    component: () => import('../views/SettingsView.vue')
  }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes
})

export default router