if (!process.env.SERP_API_KEY) {
  throw new Error('you need to specify SERP_API_KEY as env variable');
}
const SerpApi = require('google-search-results-nodejs')
const search = new SerpApi.GoogleSearch(process.env.SERP_API_KEY)
const fs = require('fs');

function checkIsHubsRoomOrScene (url) {
  // check if url has pattern https://hubs.mozilla.com/[alphanumeric]/[alphanumeric] or http://hubs.mozilla.com/scenes/
  return (url.match(/https:\/\/hubs.mozilla.com\/[a-zA-Z0-9]+\/[a-zA-Z0-9]+/) || url.includes("/scenes")) && !url.includes("/docs/")
}

const urls = fs.readFileSync('urls.txt', 'utf8').split('\n');

async function searchGoogle(query, start) {
  return new Promise( (resolve, reject) => {
    search.json({
    q: query,
    start,
    safe: "active",
    num:50
    }, (result) => {
      if (result.error) return reject(result.error);
      resolve(result);
    })
  });
}

async function run() {
  let start = 0;
  const query = "inurl:scenes site:hubs.mozilla.com";
  while (true) {
    try {
      const result = await searchGoogle(query, start);

      console.log(`start: ${start}, results: ${result.search_information.total_results}`)
      for (const r of result.organic_results) {
        const link = r.link;
        //console.log(link);

        const isHubsRoomOrScene = checkIsHubsRoomOrScene(link);
        if (isHubsRoomOrScene) {
          console.log(link);

          if (!urls.includes(link)) {
            urls.push(link);
          }
        }
      }

      start += result.organic_results.length;

      fs.writeFileSync('urls.txt', urls.join('\n'));
    } catch (e) {
      console.error(e);
      break;
    }

  }


}


run()
  .then( () => console.log('done'), (err) => console.log('error', err))
  .catch( (err) => console.log('error', err) );
