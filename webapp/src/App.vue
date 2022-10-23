<template>
  <div class="bg-slate-800 gap gap-y-4 dark:bg-slate-800 text-gray-200 p-4">
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
    <div class="rounded p-2">
      <!-- draw labels -->
      <div class="grid grid-cols-7 gap-x-4 flex-wrap gap-y-4">
        <div v-for="label in filteredLabels" :key="label" class="flex flex-row items-center justify-between gap-x gap-x-4 text-xs rounded py-1 px-2 select-none cursor-pointer hover:bg-slate-400" :class='selectedLabelClass(label)' @click='onToggleLabel(label)'>
          <div class="text-center">{{ label.label }}</div>
          <div class="text-center bg-slate-800 rounded-lg w-8 text-white">{{ label.count }}</div>
        </div>
      </div>

      <button class="mt-8 bg-slate-700 rounded py-2 px-4 select-none cursor-pointer" @click='clearLabels'>Clear</button>
    </div>




    <div class="pt-4" v-if="filteredData.length > 0">
      <div class="text-lg font-thin font-sans">Showing {{ filteredData.length }} scenes</div>
    </div>
    <div class="pt-1 grid grid-cols-4 gap gap-x-4 gap-y-2">
      <!-- show heading -->

      <div v-for='hubs in filteredData' :key='hubs.thumbnailFilename' class="bg-slate-800 rounded p-2 hover:bg-slate-600">
        <a :href="hubs.url" target="_blank">
          <div class="text-semibold">{{hubs.title}}</div>
        </a>

        <!-- draw thumbnail -->
        <div class="flex object-contain justify-center">
          <a :href="hubs.url" target="_blank">
            <img :src="hubs.thumbnail" class="w-48 h-48" loading='lazy' >
          </a>
        </div>

        <button class="text-xs mt-2 bg-slate-700 rounded py-1 px-2 select-none cursor-pointer hover:underline" @click='onToggleTopLabel(hubs.labels)'>More Like This</button>

      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, ref, onMounted } from 'vue';
import HelloWorld from './components/HelloWorld.vue';

import dataRaw from './all.json';
import { uniq, uniqBy, maxBy } from 'lodash';

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
};
const confidenceThreshold = ref(80);
const dataBefore = dataRaw as HubsEntry[];
const data = computed( () => {
  return uniqBy(dataBefore.map( hubs => {
    const labels = hubs.labels.filter( label => label.Confidence > confidenceThreshold.value );
    return {
      ...hubs,
      label: labels,
    }
  }), 'thumbnail');
});


// create a computer setter and getter that stores the labels
// in the query parameter of the url
const queryUrl = ref('');
onMounted( () => {
  queryUrl.value = window.location.search;
});
window.addEventListener('popstate', () => {
  queryUrl.value = window.location.search;
});
const selectedLabels = computed({
  get: () => {
    const urlParams = new URLSearchParams(queryUrl.value);
    const labels = urlParams.get('labels');
    console.log('read', labels);
    if (labels) {
      return labels.split(',');
    }
    return [];
  },
  set: (value) => {
    const urlParams = new URLSearchParams(window.location.search);
    urlParams.set('labels', value.join(','));
    console.log('SET', value);
    queryUrl.value = urlParams;
    window.history.pushState({}, '', '?' + urlParams);
  }
});
//const selectedLabels = ref([] as string[]);

const filteredData = computed(() => {
  const anyLabelSelected = selectedLabels.value.length > 0;
  return data.value.filter((d) => {
    const allSelectedLabelsPresent = selectedLabels.value.every((label) => {
      return d.labels.some((l) => l.Name === label && l.Confidence > confidenceThreshold.value);
    });
    return !anyLabelSelected || allSelectedLabelsPresent;
  });
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
  return labelsFilter;
});
const filteredLabels = computed( () => {
  return calculatedLabels.value.filter((label) => label.count > 1);
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
};

const onToggleTopLabel = (labels: LabelsEntry[]) => {
  console.log('check top label', labels);

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
};

const selectedLabelClass = (label: LabelFilter) => {
  return {
    'bg-slate-400': label.selected,
    'bg-slate-700': !label.selected,
  };
};

const clearLabels = () => {
  selectedLabels.value = [];
};


</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
</style>
