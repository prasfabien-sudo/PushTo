# Third-party software and data

# Third-party services

## Astrometry.net

The Android app can submit user-selected images to the Astrometry.net web API at [nova.astrometry.net](https://nova.astrometry.net/). No Astrometry.net solver library or index data is bundled in the app. Image submission is initiated only after the user confirms it in the app; the request asks the service to keep the submission non-public (`publicly_visible=n`). The service's own terms and data-retention practices apply.

The Android app source is distributed under GPL-3.0-or-later; see [LICENSE](./LICENSE).
