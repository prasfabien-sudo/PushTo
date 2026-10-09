#ifndef ICONS_H
#define ICONS_H

#include <pgmspace.h>

const char HTTP_ICONS[] PROGMEM = R"rawliteral(
<svg xmlns="http://www.w3.org/2000/svg">
  <symbol id="icon-telescope" viewBox="0 0 24 24"><path d="m10.5 20.5 3-7"/><path d="m8.5 17.5 5 2"/><path d="m14 13 3 1"/><path d="m17 14 3-8-5-2-4 7"/><path d="m12 11-7 3 2 4 7-3"/></symbol>
  <symbol id="icon-crosshair" viewBox="0 0 24 24"><circle cx="12" cy="12" r="8"/><path d="M12 2v4m0 12v4M2 12h4m12 0h4"/></symbol>
  <symbol id="icon-settings" viewBox="0 0 24 24"><path d="M12 8a4 4 0 1 0 0 8 4 4 0 0 0 0-8Z"/><path d="m19.4 15 .1.1 1.4 1.1-1.4 2.4-1.7-.7a8 8 0 0 1-1.8 1l-.3 1.8h-2.8l-.3-1.8a8 8 0 0 1-1.8-1l-1.7.7-1.4-2.4 1.4-1.1a8 8 0 0 1 0-2l-1.4-1.1 1.4-2.4 1.7.7a8 8 0 0 1 1.8-1l.3-1.8h2.8l.3 1.8a8 8 0 0 1 1.8 1l1.7-.7 1.4 2.4-1.4 1.1a8 8 0 0 1 0 2Z" transform="translate(-1 -1)"/></symbol>
  <symbol id="icon-moon" viewBox="0 0 24 24"><path d="M20.9 13A9 9 0 0 1 11 3.1 9 9 0 1 0 20.9 13Z"/></symbol>
  <symbol id="icon-more-horizontal" viewBox="0 0 24 24"><circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="12" r="1"/></symbol>
  <symbol id="icon-gamepad-2" viewBox="0 0 24 24"><path d="M6 11h4m-2-2v4"/><path d="M15 12h.01m3-2h.01"/><path d="M7 6h10a4 4 0 0 1 3.9 3.1l1 4.4a3 3 0 0 1-5 2.9l-2-1.9H9l-2 1.9a3 3 0 0 1-5-2.9l1-4.4A4 4 0 0 1 7 6Z"/></symbol>
  <symbol id="icon-book-open" viewBox="0 0 24 24"><path d="M12 7v14"/><path d="M3 18V5a1 1 0 0 1 1-1h3a5 5 0 0 1 5 5 5 5 0 0 1 5-5h3a1 1 0 0 1 1 1v13a1 1 0 0 1-1 1h-4a4 4 0 0 0-4 2 4 4 0 0 0-4-2H4a1 1 0 0 1-1-1Z"/></symbol>
  <symbol id="icon-flask-conical" viewBox="0 0 24 24"><path d="M9 3h6m-5 0v7l-5.5 8.5A2 2 0 0 0 6.2 21h11.6a2 2 0 0 0 1.7-2.5L14 10V3"/><path d="M8 16h8"/></symbol>
  <symbol id="icon-clipboard-list" viewBox="0 0 24 24"><rect x="5" y="4" width="14" height="18" rx="2"/><path d="M9 4.5V3h6v1.5M9 10h6m-6 4h6m-6 4h3"/></symbol>
  <symbol id="icon-map-pin" viewBox="0 0 24 24"><path d="M20 10c0 5-8 12-8 12S4 15 4 10a8 8 0 1 1 16 0Z"/><circle cx="12" cy="10" r="2.5"/></symbol>
  <symbol id="icon-clock-3" viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/></symbol>
  <symbol id="icon-sparkles" viewBox="0 0 24 24"><path d="m12 3 1.8 5.2L19 10l-5.2 1.8L12 17l-1.8-5.2L5 10l5.2-1.8L12 3Z"/><path d="m19 14 .9 2.1L22 17l-2.1.9L19 20l-.9-2.1L16 17l2.1-.9L19 14ZM5 3l.7 1.8L7.5 5.5l-1.8.7L5 8l-.7-1.8-1.8-.7 1.8-.7L5 3Z"/></symbol>
  <symbol id="icon-planet" viewBox="0 0 24 24"><circle cx="12" cy="12" r="5"/><path d="M3 14c3-3 8-5 13-5s7 1 6 3-6 4-12 5-9 0-8-2c.2-.4.5-.8 1-1Z"/></symbol>
  <symbol id="icon-orbit" viewBox="0 0 24 24"><circle cx="12" cy="12" r="2"/><ellipse cx="12" cy="12" rx="10" ry="4" transform="rotate(-35 12 12)"/><circle cx="19" cy="6" r="1"/></symbol>
  <symbol id="icon-cloud" viewBox="0 0 24 24"><path d="M20 16.2A4.8 4.8 0 0 0 18 7h-1.2A7 7 0 0 0 3 10a4.5 4.5 0 0 0 1 8h14a4.5 4.5 0 0 0 2-1.8Z"/></symbol>
  <symbol id="icon-star" viewBox="0 0 24 24"><path d="m12 3 2.8 5.7 6.2.9-4.5 4.4 1.1 6.2-5.6-3-5.6 3 1.1-6.2L3 9.6l6.2-.9L12 3Z"/></symbol>
  <symbol id="icon-circle-check" viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="m8 12 2.5 2.5L16 9"/></symbol>
  <symbol id="icon-circle-x" viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="m9 9 6 6m0-6-6 6"/></symbol>
  <symbol id="icon-compass" viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="m16.2 7.8-2.8 6.4-6.4 2.8 2.8-6.4 6.4-2.8Z"/></symbol>
  <symbol id="icon-ruler" viewBox="0 0 24 24"><path d="m21.3 8.7-6-6a2.4 2.4 0 0 0-3.4 0l-9.2 9.2a2.4 2.4 0 0 0 0 3.4l6 6a2.4 2.4 0 0 0 3.4 0l9.2-9.2a2.4 2.4 0 0 0 0-3.4Z"/><path d="m7.5 10.5 2 2m1-5 2 2m1-5 2 2m-1 9 2 2"/></symbol>
  <symbol id="icon-trash-2" viewBox="0 0 24 24"><path d="M3 6h18m-2 0-.9 14H5.9L5 6m4 0V4h6v2m-5 4v6m4-6v6"/></symbol>
  <symbol id="icon-plus" viewBox="0 0 24 24"><path d="M12 5v14m-7-7h14"/></symbol>
  <symbol id="icon-save" viewBox="0 0 24 24"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2Z"/><path d="M17 21v-8H7v8M7 3v5h8"/></symbol>
  <symbol id="icon-lightbulb" viewBox="0 0 24 24"><path d="M9 18h6m-5 4h4m-2-20a7 7 0 0 0-4 12.7c.6.4 1 1 1 1.8h6c0-.8.4-1.4 1-1.8A7 7 0 0 0 12 2Z"/></symbol>
  <symbol id="icon-play" viewBox="0 0 24 24"><path d="m8 5 12 7-12 7V5Z"/></symbol>
  <symbol id="icon-globe-2" viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M2 12h20M12 2a15 15 0 0 1 0 20M12 2a15 15 0 0 0 0 20"/></symbol>
</svg>
)rawliteral";

#endif
