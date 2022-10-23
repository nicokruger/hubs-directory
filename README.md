# Hubs Directory

A browsable and searchable community built directory of Mozilla Hubs scenes and rooms. Find hidden gems and get inspired.

The main Directory is hosted at [https://hubs.inmytree.co.za](https://hubs.inmytree.co.za).

### Help Needed!
To add a new URL for the directory to crawl, send a Merge Request on an edit to [crawler/urls.txt](https://github.com/nicokruger/hubs-directory/edit/main/crawler/urls.txt)

## Overview

The lack of a way to discover [Hubs](https://hubs.mozilla.com) Rooms (and any type of WebXR rooms in general) has been quite surprising to me. This project aims to be a searchable catalog/directory for public Hubs Rooms (and other open XR platforms). It should provide an easy way to explore and discover new content in VR and devices.

## How it works

The current version can index [Mozzila Hubs Rooms](https://hubs.mozilla.com). It works from a central URL master list. This URL master list can be populated either by hand or by an included Google Crawler that searches the web for public room links. The Google Crawler uses [SerpApi](https://serpapi.com/).

After crawling for new pages, a seperate script is used that uses [Amazon Rekognition](https://aws.amazon.com/rekognition/) is used to gather some labels from the thumbnail image. It is these labels and additional metadata that will form the basis of the search and the discovery through the catalog.

## Setup

### Master URL List

To add a new URL for the main site to crawl, send a Merge Request on an edit to [crawler/urls.txt](https://github.com/nicokruger/hubs-directory/edit/main/crawler/urls.txt)

### Crawler

The crawler is a NodeJS 16 project. Run:

```
npm install
AWS_PROFILE=<aws profile in our credentials> node crawler.js
```
This should update data/all.json which you can the copy over to the webapp src/ directory. Alternatively, you can use of course use AWS_ACCESS_KEY_ID etc. if you do have only a single AWS account.

### Webapp

The webapp uses Vue 3 + tailwindcss. Inside the webapp directory, run:

```
npm install
npm run serve
```
Then navigate to http://localhost:8080

All data (for now) is self contained (there is no backend) so it should be relatively painless to make changes and test them locally.
