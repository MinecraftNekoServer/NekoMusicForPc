<template>
  <div class="playlists-view">
    <MusicList title="我的歌单" :show-search="false" :show-favorite="true" />
  </div>
</template>

<script setup>
import MusicList from '../components/MusicList.vue'
</script>

<style scoped>
.playlists-view {
  height: 100%;
}
</style>