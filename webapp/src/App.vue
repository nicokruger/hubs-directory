<template>
  <div class="bg-slate-800 gap gap-y-4 text-gray-200 overflow-x-none lg:p-4">
    <!-- put a link to the github repo at the top right -->
    <div class="flex justify-end">
      <a
        href="https://github.com/nicokruger/hubs-directory"
        class="text-gray-400 hover:text-gray-200"
        target="_blank"
      >
      <img src="https://github.githubassets.com/images/modules/logos_page/GitHub-Mark.png" width="32" height="32" alt="GitHub" />


      </a>
    </div>

    <div class="text-lg font-semibold font-sans">Hubs Directory</div>
    <div class="text-gray-400">
    A browsable and searchable community built directory of Mozilla Hubs scenes and rooms. Find hidden gems and get inspired.
    </div>

    <div class="mt-4 flex flex-col items-center justify-end gap-x gap-x-6">
      <div>
        <span color="text-white">{{dataRaw.length}}</span> rooms indexed.
      </div>
      <div class="text-gray-400 text-right text-xs font-thin">
      Help us build this directory by helping us expand the <a href="https://github.com/nicokruger/hubs-directory/edit/main/crawler/urls.txt" class="underline" target="_blank">directory</a>
      </div>
    </div>


    <div class="mt-8 text-lg font-thin font-sans">Categories</div>
    <div class="p-2 gap gap-x-4">
      <!-- draw labels -->
      <div class="grid grid-cols-4 lg:grid-cols-5 xl:grid-cols-7 gap-x-4 flex-wrap gap-y-4">
        <div v-for="label in filteredLabels.slice(0,showLabelsLimit)" :key="label" class="flex flex-wrap flex-row items-center justify-between gap-x gap-x-4 text-xs rounded py-1 px-1 select-none cursor-pointer hover:bg-slate-400" :class='selectedLabelClass(label)' @click='onToggleLabel(label)'>
          <div class="text-center text-xs ">{{ label.label }}</div>
          <div class="text-center bg-slate-800 rounded-lg text-white truncate">{{ label.count }}</div>
        </div>

        <button v-if='showLabelsLimit < filteredLabels.length'
                class="bg-slate-500 rounded py-1 px-4 text-white select-none cursor-pointer hover:text-black hover:bg-slate-300" @click='showLabelsLimit += 40'>{{filteredLabels.length - showLabelsLimit}} More</button>

      </div>

      <div class="flex flex-row gap gap-x-4 justify-center items-center">
        <button class="mt-8 bg-slate-700 rounded py-2 px-4 select-none cursor-pointer" @click='clearLabels'>Clear</button>
      </div>
    </div>




    <div class="pt-4" v-if="filteredData.length > 0">
      <div class="text-lg font-thin font-sans">Showing {{ filteredData.length }} scenes</div>
        <button class="mt-8 bg-slate-700 rounded py-2 px-4 select-none cursor-pointer" @click='openRandomScene'>Random</button>
    </div>
    <div class="pt-1 grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap gap-x-4 gap-y-4">
      <!-- show heading -->

      <div v-for='hubs in filteredData.slice(0,showLimit)' :key='hubs.thumbnailFilename' class="bg-slate-800 rounded p-2 hover:bg-slate-600 relative border border-slate-700">
        <a :href="hubs.url" target="_blank">
          <div class="text-semibold break-words">{{hubs.title}}</div>
        </a>

        <!-- add a new label at the top right if the scene is new -->
        <div v-if="hubs.isNew" class="bg-slate-700 rounded-lg text-white text-xs px-1 py-1 absolute top-0 right-0">New</div>

        <!-- draw thumbnail -->
        <div class="flex object-contain justify-center">
          <a :href="hubs.url" target="_blank">
            <img :src="hubs.thumbnail" class="w-48 h-48" loading='lazy' >
          </a>
        </div>

        <button class="text-xs mt-2 bg-slate-700 rounded py-1 px-2 select-none cursor-pointer hover:underline" @click='onToggleTopLabel(hubs.labels)'>More Like This</button>

        <!-- show domain at bottom right -->
        <div class="text-xs text-gray-400 absolute bottom-0 right-0">{{hubs.urlDomain}}</div>

        <!--
        <div>
          <div class="flex flex-col">
            <div v-for='(label,i) in hubs.labels' :key='i'>{{label.Name}} - {{label.Confidence}}</div>
          </div>
        </div>
        -->

      </div>

    </div>

    <div v-show='showLimit < filteredData.length' id='loadmore' class="bg-slate-800 rounded p-2 hover:bg-slate-600 relative border border-slate-700">
      <button class="text-semibold break-words" @click='onLoadMore'>Load More</button>
    </div>

    <div class="h-32"></div>

  </div>
</template>

<script setup lang="ts">
import { computed, ref, onMounted } from 'vue';
import HelloWorld from './components/HelloWorld.vue';

import { uniq, uniqBy, maxBy, sortBy } from 'lodash';
import dataRaww from './all.json';
const dataRaw = uniqBy(dataRaww, 'thumbnail');

type LabelFilter = {
  label: string;
  selected: boolean;
  count: number;
};
type LabelsEntry = {
  Name: string;
  Confidence: number;
};
type HubsEntry = {
  url: string;
  title: string;
  labels: LabelsEntry[];
  links: string[];
};
const confidenceThreshold = ref(40);
const dataBefore = dataRaw as HubsEntry[];
const data = computed( () => {
  return uniqBy(dataBefore
    .map( hubs => {
      const labels = hubs.labels.filter( label => label.Confidence > confidenceThreshold.value );
      return {
        ...hubs,
        label: labels,
      }
    }), 'thumbnail');
});


const queryUrl = ref('');
const pageSize = 30;
const showLimitUrl = ref(pageSize);
const showLabelsLimitUrl = ref(30);

onMounted( () => {
  queryUrl.value = window.location.search;
  showLimitUrl.value = parseInt(window.location.showLimit ?? pageSize);
  showLabelsLimitUrl.value = parseInt(window.location.showLabelsLimit ?? 30);

});
window.addEventListener('popstate', () => {
  queryUrl.value = window.location.search;
  showLimitUrl.value = parseInt(window.location.showLimit ?? pageSize);
  showLabelsLimitUrl.value = parseInt(window.location.showLabelsLimit ?? 30);
});



const selectedLabels = computed({
  get: () => {
    const urlParams = new URLSearchParams(queryUrl.value);
    const labels = urlParams.get('labels');
    if (labels) {
      return labels.split(',');
    }
    return [];
  },
  set: (value) => {
    const urlParams = new URLSearchParams(window.location.search);
    urlParams.set('labels', value.join(','));
    queryUrl.value = urlParams;
    window.history.pushState({}, '', '?' + urlParams);
  }
});


const showLimit = computed({
  get: () => {
    const urlParams = new URLSearchParams(queryUrl.value);
    const showLimit = urlParams.get('showLimit');
    if (showLimit) {
      return parseInt(showLimit);
    }
    return pageSize;
  },
  set: (value) => {
    const urlParams = new URLSearchParams(window.location.search);
    urlParams.set('showLimit', value.toString());
    queryUrl.value = urlParams;
    window.history.replaceState({}, '', '?' + urlParams);
  }
});
const showLabelsLimit = computed({
  get: () => {
    const urlParams = new URLSearchParams(queryUrl.value);
    const showLabelsLimit = urlParams.get('showLabelsLimit');
    if (showLabelsLimit) {
      return parseInt(showLabelsLimit);
    }
    return showLabelsLimitUrl.value;
  },
  set: (value) => {
    const urlParams = new URLSearchParams(window.location.search);
    urlParams.set('showLabelsLimit', value.toString());
    queryUrl.value = urlParams;
    window.history.replaceState({}, '', '?' + urlParams);
  }
});
//const selectedLabels = ref([] as string[]);

const filteredData = computed(() => {
  const anyLabelSelected = selectedLabels.value.length > 0;
  const maxAddedTime = maxBy(data.value, 'addedTime');
  const scenes = data.value.filter((d) => {
    const allSelectedLabelsPresent = selectedLabels.value.every((label) => {
      return d.labels.some((l) => l.Name === label && l.Confidence > confidenceThreshold.value);
    });
    return !anyLabelSelected || allSelectedLabelsPresent;
  }).map( (scene) => {
    if (!scene.addedTime) {
      scene.addedTime = new Date('1970-01-01').toISOString();
    }
    if (!scene.updatedTime) {
      scene.updatedTime = new Date('1970-01-01').toISOString();
    }
    // if within the last 24 hours from max addedTime, label as new
    scene.isNew = new Date(scene.addedTime).getTime() > new Date(maxAddedTime.addedTime).getTime() - 24 * 60 * 60 * 1000;

    const urlDomain = new URL(scene.url).hostname;
    scene.urlDomain = urlDomain;
    return scene;
  });
  // newest added first
  return sortBy(scenes, 'addedTime').reverse();
});

const calculatedLabels = computed( () => {
  const labels = new Set<string>();
  for (const hubs of data.value) {
    for (const label of hubs.labels) {
      if (label.Confidence > confidenceThreshold.value) {
        labels.add(label.Name);
      }
    }
  }
  const labelsArray = Array.from(labels);
  labelsArray.sort();

  const labelsFilter: LabelFilter[] = [];
  for (const label of labelsArray) {
    const count = filteredData.value.filter((hubs) => hubs.labels.some((l) => l.Name === label && l.Confidence > confidenceThreshold.value )).length;
    const selected = selectedLabels.value.includes(label);
    labelsFilter.push({
      label,
      selected,
      count
    });
  }
  return sortBy(labelsFilter,'count').reverse();
});
const filteredLabels = computed( () => {
  return calculatedLabels.value.filter((label) => label.count > 0);
});

const onToggleLabel = (label: LabelFilter) => {
  const labelKey = label.label;

  let labels = [...selectedLabels.value];
  if (labels.includes(labelKey)) {
    labels = labels.filter((l) => l !== labelKey);
  } else {
    labels.push(labelKey);
  }
  selectedLabels.value = labels;

  showLimit.value = pageSize;
};

const onToggleTopLabel = (labels: LabelsEntry[]) => {

  const labelFilters = labels.map( label => {
    return filteredLabels.value.find( l => l.label === label.Name );
  }).filter( l => l !== undefined ).map( l => l.label );
  const eligibleLabelFilters = labels.filter( l => labelFilters.includes(l.Name) );
  const topLabel = maxBy(eligibleLabelFilters, 'Confidence');

  let newLabels = [...selectedLabels.value];
  const labelKey = topLabel?.Name;
  newLabels.push(labelKey);
  newLabels = uniq(newLabels);
  selectedLabels.value = newLabels;

  showLimit.value = pageSize;
};

const selectedLabelClass = (label: LabelFilter) => {
  return {
    'bg-slate-400': label.selected,
    'bg-slate-700': !label.selected,
  };
};

const clearLabels = () => {
  selectedLabels.value = [];
  showLimit.value = pageSize;
  showLabelsLimit.value = 30;
};


const openRandomScene = () => {
  const randomIndex = Math.floor(Math.random() * filteredData.value.length);
  const hubs = filteredData.value[randomIndex];
  window.open(hubs.url, '_blank');
};

const onLoadMore = () => {
  showLimit.value += pageSize;
};

onMounted( () => {

  // setup an intersection observer on #loadmore
  const loadMore = document.getElementById('loadmore');
  if (loadMore) {
    const observer = new IntersectionObserver((entries) => {
      if (entries[0].isIntersecting) {
        onLoadMore();
      }
    });
    observer.observe(loadMore);
  }


});

</script>

<style>
body {
}
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
html {
}
</style>
