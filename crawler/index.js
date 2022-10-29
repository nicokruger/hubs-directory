const AWS = require('aws-sdk');
const Rekognition = new AWS.Rekognition();
const fetch = require('node-fetch');
const cheerio = require('cheerio');
const fs = require('fs');
const child_process = require('child_process');

const HUBS_VERSION = 4;

function getProcessStdout(cmd, args) {
  const child = child_process.spawnSync(cmd, args, { encoding : 'utf8' });
  //console.log("Process finished.");
  if(child.error) {
    console.log("ERROR: ",child.error);
    throw child.error;
  }
  //console.log("stdout: ",child.stdout);
  //console.log("stderr: ",child.stderr);
  //console.log("exist code: ",child.status);
  return child.stdout;

}



async function getImageTags(imgPath) {
  const params = {
    Image: {
      Bytes: fs.readFileSync(imgPath)
    },
    MaxLabels: 20,
    MinConfidence: 70
  };
  const data = await Rekognition.detectLabels(params).promise();

  return data.Labels.map( ({Name,Confidence}) => ({Name,Confidence}) );
}

async function parseOgData(body) {
  const $ = cheerio.load(body);
  const ogImage = $('meta[property="og:image"]').attr('content');
  const ogTitle = $('meta[property="og:title"]').attr('content');
  return {ogImage,ogTitle};
}

async function getCachedIndex(url) {
  const indexFile = `./index/${url.replace(/[^a-z0-9\.]/gi,'_')}`;
  if (fs.existsSync(indexFile)) {
    return JSON.parse(fs.readFileSync(indexFile));
  }
  return false;
}
async function getHubsData(url) {

  let addedTime = new Date().toISOString();

  const dataFile = `./data/${url.replace(/[^a-z0-9]/gi,'_')}.json`;
  if (fs.existsSync(dataFile)) {
    const version = JSON.parse(fs.readFileSync(dataFile)).version;
    if (version === HUBS_VERSION) {
      return JSON.parse(fs.readFileSync(dataFile));
    } else {
      console.log('ignore older cached version', version, 'vs', url);
      addedTime = JSON.parse(fs.readFileSync(dataFile)).addedTime ?? addedTime;
    }
  }
  try {
    const response = await fetch(url);
    const headers = response.headers.raw();
    const hubEntityTypeHeader = headers['hub-entity-type'] ?? [];
    const hubNameHeader = headers['hub-name'] ?? [];

    if (!hubEntityTypeHeader.length) {
      console.error(url, 'is not a hubs room');
      return false;
    }

    const hubEntityType = hubEntityTypeHeader && hubEntityTypeHeader[0];
    const hubName = hubNameHeader && hubNameHeader[0];

    const body = await response.text();

    const {ogImage,ogTitle} = await parseOgData(body);
    //console.log('ogImage', ogImage);
    if (!ogImage) {
      console.error(`No og:image found for ${url}`);
      return false;
    }

    const ogImageFilename = ogImage.split('/').pop();
    const localImagePath = `./images/${ogImageFilename}`;

    if (!fs.existsSync(localImagePath)) {
      //console.log('download image');
      const ogImageResponse = await fetch(ogImage);
      const ogImageBuffer = await ogImageResponse.buffer();
      fs.writeFileSync(localImagePath, ogImageBuffer);
    }

    const labels = await getImageTags(localImagePath);

    const sceneModels = await getSceneModels(url);
    const links = [];
    if (!sceneModels) {
      console.log('TODO: get scene model for room');
    } else {
      //console.log('sceneModels', JSON.stringify(sceneModels,0,2));

      for (const sceneModel of sceneModels) {
        //console.log('SCENE MODEL', JSON.stringify(sceneModel,0,2));
        for (const scene of sceneModel.scenes) {
          //console.log('model_url', scene.model_url);
          const sceneFile = await getSceneModel(scene.model_url);
          //console.log('sceneFile', sceneFile);

          console.log('parse links');
          const sceneModelLinks = await getLinks(sceneFile);
          console.log('   parsed links');
          //console.log(links);
          for (const link of sceneModelLinks) {
            console.log('add room link', link);
            links.push(link);
          }

        }


      }
    }


    console.log('addedTime', addedTime);
    const sceneData = {
      url,
      thumbnailFilename: ogImageFilename,
      thumbnail: ogImage,
      title:ogTitle,
      labels,
      addedTime,
      updatedTime:new Date().toISOString(),
      hubEntityType,
      hubName,
      links,
      version: HUBS_VERSION
    }
    fs.writeFileSync(dataFile, JSON.stringify(sceneData));
    return sceneData;
  } catch (e) {
    console.error(`cannot parse ${url}`, e);
    return false;
  }

}

async function fetchSceneModel(sceneId, scenes=[]) {
  const response = await fetch(`https://hubs.mozilla.com/api/v1/scenes/${sceneId}`);
  const data = await response.json();

  scenes.push(data);

  for (const scene of data.scenes) {
    if (scene.parent_scene_id) {
      await fetchSceneModel(scene.parent_scene_id, scenes);
    }
  }
  return scenes;
}

async function getSceneModels(url) {
  if (!url.includes('scene')) {
    return false;
  }

  const sceneModelsFile = `./scenes/sceneModels___${url.replace(/[^a-z0-9]/gi,'_')}.json`;
  console.log('smf', sceneModelsFile);
  if (fs.existsSync(sceneModelsFile)) {
    return JSON.parse(fs.readFileSync(sceneModelsFile));
  }

  // capture sceneId from url path parameter https://host/scenes/:sceneId/:anything
  const sceneId = url.split('/').filter(Boolean)[3];
  //console.log('sceneId', sceneId);

  const allScenes = await fetchSceneModel(sceneId);
  fs.writeFileSync(sceneModelsFile, JSON.stringify(allScenes));

  return allScenes;
}

async function getSceneModel(url) {
  const fileKey = `./scenes/${url.split('/').pop()}`;
  console.log('look for scene file', fileKey);
  if (fs.existsSync(fileKey)) {
    console.log('  cached', fileKey);
    return fileKey;
  }

  const response = await fetch(url);
  const body = await response.buffer();
  fs.writeFileSync(fileKey, body);

  return fileKey;
}
async function getLinks(sceneFile) {
  const stdout = getProcessStdout("./a.out", [sceneFile]);
  const links = stdout.split('\n').filter(Boolean);
  //console.log('links', links);
  return links;
}
async function main() {
  const urls = fs.readFileSync('urls.txt', 'utf8').split('\n');
  const processedUrls = [];

  const scenes = [];
  let processed = 0;
  while (urls.length) {
    const url = urls.pop();
  //for (const url of urls) {
    if (!url.trim()) {
      continue;
    }
    if (url.startsWith('#')) {
      continue;
    }
    console.log(`[${processed}/${urls.length}] parse url ${url}`);

    const sceneData = await getHubsData(url);
    if (!sceneData) {
      console.log('skip', url);
      continue;
    }

    for (const link of sceneData.links) {
      if (!urls.includes(link) && !processedUrls.includes(link)) {
        console.log('add room link', link);
        urls.push(link);
      } else {
        console.log('already grabbed', link);
      }
    }

    scenes.push(sceneData);
    processedUrls.push(url);
    processed++;

  }

  const allFile = `./data/all.json`;
  fs.writeFileSync(allFile, JSON.stringify(scenes));
}

main()
  .then(() => console.log('Done'))
  .catch((err) => console.error(err));
