# wchowellarchive Portfolio Site

Status: Active portfolio/site project

wchowellarchive is a Firebase-hosted engineering portfolio and project archive used to present Carter Howell's resume, project documentation, and technical work. The repo includes the static site source, Firebase configuration, project pages, and public media used by the portfolio.

![Portfolio homepage preview](media/MainPage.png)

## Overview

The site organizes engineering and software projects into a public archive. It acts as both a portfolio and a resume companion, with project pages, resume links, SEO metadata, and Firebase hosting.

## Implementation

The portfolio is built with HTML, CSS, JavaScript, and Firebase Hosting. Project pages use shared styling, metadata, and structured descriptions so the archive can grow as new projects mature.

## Repository Structure

- `public/`: static portfolio pages, shared CSS/JavaScript, project pages, and public media
- `firebase.json`: Firebase Hosting configuration
- `firestore.rules`: Firestore rules used by the project
- `firestore.indexes.json`: Firestore index configuration
- `package.json`: project scripts and tooling metadata

## Deployment Config

The public repo uses placeholder Firebase web configuration in `public/firebase.js`. Replace those values with the deployment project's Firebase web app config before deploying from a fresh checkout.

## Role In The Portfolio

This project is supporting infrastructure. It matters because it connects the public story together, but the strongest visible work should still be the engineering projects themselves: PCB design, embedded systems, robotics, and power electronics.

## Future Improvements

- Add GitHub links on each project page.
- Add stronger project filtering by hardware/software/status.
- Add more diagrams and measured results for engineering projects.
- Keep older software/game projects visibly secondary to engineering work.

## Live Site

[wchowellarchive.web.app](https://wchowellarchive.web.app)
