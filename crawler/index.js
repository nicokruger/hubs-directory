const AWS = require('aws-sdk');
const Rekognition = new AWS.Rekognition();
const fetch = require('node-fetch');
const cheerio = require('cheerio');
const fs = require('fs');

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

async function getHubsData(url) {
  const dataFile = `./data/${url.replace(/[^a-z0-9]/gi,'_')}.json`;
  if (fs.existsSync(dataFile)) {
    return JSON.parse(fs.readFileSync(dataFile));
  }
  const response = await fetch(url);
  const body = await response.text();

  const {ogImage,ogTitle} = await parseOgData(body);
  console.log('ogImage', ogImage);
  if (!ogImage) {
    throw new Error(`No og:image found for ${url}`);
  }

  const ogImageFilename = ogImage.split('/').pop();
  const localImagePath = `./images/${ogImageFilename}`;

  if (!fs.existsSync(localImagePath)) {
    console.log('download image');
    const ogImageResponse = await fetch(ogImage);
    const ogImageBuffer = await ogImageResponse.buffer();
    fs.writeFileSync(localImagePath, ogImageBuffer);
  }

  const labels = await getImageTags(localImagePath);

  const sceneData = {
    url,
    thumbnailFilename: ogImageFilename,
    thumbnail: ogImage,
    title:ogTitle,
    labels,
    addedTime:new Date().toISOString(),
    updatedTime:new Date().toISOString()
  }
  fs.writeFileSync(dataFile, JSON.stringify(sceneData));
  return sceneData;

}


async function main() {
  const urls = fs.readFileSync('urls.txt', 'utf8').split('\n');

  const scenes = [];
  for (const url of urls) {
    if (!url.trim()) {
      continue;
    }
    if (url.startsWith('#')) {
      continue;
    }
    console.log('parse url', url);

    const sceneData = await getHubsData(url);

    scenes.push(sceneData);

  }

  const allFile = `./data/all.json`;
  fs.writeFileSync(allFile, JSON.stringify(scenes));
}

main()
  .then(() => console.log('Done'))
  .catch((err) => console.error(err));
