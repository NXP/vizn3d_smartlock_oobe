"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[671],{3905:function(e,t,n){n.d(t,{Zo:function(){return p},kt:function(){return u}});var r=n(7294);function o(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function a(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);t&&(r=r.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,r)}return n}function i(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?a(Object(n),!0).forEach((function(t){o(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):a(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function c(e,t){if(null==e)return{};var n,r,o=function(e,t){if(null==e)return{};var n,r,o={},a=Object.keys(e);for(r=0;r<a.length;r++)n=a[r],t.indexOf(n)>=0||(o[n]=e[n]);return o}(e,t);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);for(r=0;r<a.length;r++)n=a[r],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(o[n]=e[n])}return o}var l=r.createContext({}),s=function(e){var t=r.useContext(l),n=t;return e&&(n="function"==typeof e?e(t):i(i({},t),e)),n},p=function(e){var t=s(e.components);return r.createElement(l.Provider,{value:t},e.children)},d={inlineCode:"code",wrapper:function(e){var t=e.children;return r.createElement(r.Fragment,{},t)}},m=r.forwardRef((function(e,t){var n=e.components,o=e.mdxType,a=e.originalType,l=e.parentName,p=c(e,["components","mdxType","originalType","parentName"]),m=s(n),u=o,h=m["".concat(l,".").concat(u)]||m[u]||d[u]||a;return n?r.createElement(h,i(i({ref:t},p),{},{components:n})):r.createElement(h,i({ref:t},p))}));function u(e,t){var n=arguments,o=t&&t.mdxType;if("string"==typeof e||o){var a=n.length,i=new Array(a);i[0]=m;var c={};for(var l in t)hasOwnProperty.call(t,l)&&(c[l]=t[l]);c.originalType=e,c.mdxType="string"==typeof e?e:o,i[1]=c;for(var s=2;s<a;s++)i[s]=n[s];return r.createElement.apply(null,i)}return r.createElement.apply(null,n)}m.displayName="MDXCreateElement"},9881:function(e,t,n){n.r(t),n.d(t,{frontMatter:function(){return c},contentTitle:function(){return l},metadata:function(){return s},toc:function(){return p},default:function(){return m}});var r=n(7462),o=n(3366),a=(n(7294),n(3905)),i=["components"],c={sidebar_position:1},l="Introduction",s={unversionedId:"intro",id:"intro",isDocsHomePage:!1,title:"Introduction",description:"Welcome to the Developer Guide for the SLN-VIZN3D-IOT!",source:"@site/docs/intro.md",sourceDirName:".",slug:"/intro",permalink:"/vizn3d_smartlock_oobe/docs/intro",tags:[],version:"current",sidebarPosition:1,frontMatter:{sidebar_position:1},sidebar:"tutorialSidebar",next:{title:"Setup and Installation",permalink:"/vizn3d_smartlock_oobe/docs/setup_and_install"}},p=[{value:"Smart Lock Application Layout",id:"smart-lock-application-layout",children:[],level:2}],d={toc:p};function m(e){var t=e.components,c=(0,o.Z)(e,i);return(0,a.kt)("wrapper",(0,r.Z)({},d,c,{components:t,mdxType:"MDXLayout"}),(0,a.kt)("h1",{id:"introduction"},"Introduction"),(0,a.kt)("p",null,"Welcome to the Developer Guide for the SLN-VIZN3D-IOT!"),(0,a.kt)("p",null,"The purpose of this guide is to help developers gain a better understanding of the software design and architecture of the Smart Lock application in order to more easily and efficiently implement applications using the SLN-VIZN3D-IOT."),(0,a.kt)("p",null,"This guide will cover topics including the ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/bootloader/intro"},"Bootloader"),", the ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/intro"},"Framework + HAL Architecture Design"),", as well as the project-specific features of the ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/smart_lock/intro"},"Smart Lock")," application which may be relevant to developing Machine Vision applications for the SLN-VIZN3D-IOT."),(0,a.kt)("h2",{id:"smart-lock-application-layout"},"Smart Lock Application Layout"),(0,a.kt)("p",null,"The Smart Lock Application for the SLN-VIZN3D-IOT provides a fully integrated HW + SW solution which allows\nfor the rapid prototyping and development of Machine Vision-based applications.\nThe Smart Lock application comes with full source code as well as hardware reference designs\nto help get developers up and running as quickly as possible."),(0,a.kt)("p",null,"The design of the Smart Lock app is broken into two distinct layers:\nan underlying ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/intro"},'"Framework + HAL"')," layer,\nand a top-level ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/smart_lock/intro"},'"Application"')," layer."),(0,a.kt)("p",null,(0,a.kt)("img",{alt:"Smart Lock Software Diagram",src:n(642).Z})),(0,a.kt)("p",null,"The bottom ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/intro"},"Framework + HAL")," layer acts as a message routing system which allows the\nperipherals connected to the board to interact with one another."),(0,a.kt)("p",null,'The Framework was designed with code portability in mind,\nwith the idea that low-level driver bindings would connect to higher-level,\nplatform-agnostic "Hardware Abstraction Layer drivers" which do not depend on\nthe underlying pin assignments, etc. which are specific to the board.\nThis design should allow for the easy migration from one platform to another,\nhelping alleviate platform lock-in\nand make code easier to read, write, modify, and maintain.'),(0,a.kt)("p",null,"The top ",(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/smart_lock/intro"},'"Application" layer'),' contains all application-specific code\nincluding the various sounds, icons, UI elements, etc.\nIn addition,\nthe "Application" layer will register all the devices relevant to the application,\nas well as their ',(0,a.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/events/event_handlers"},"Event Handlers")," which react to events triggered by other devices."),(0,a.kt)("p",null,'Separating the "Application" and "Framework + HAL" layers from each other encourages code reuse between different projects\nbecause the underlying Framework code can be reused in almost its entirety,\nwhile primarily only the Application layer code will need modifying.'),(0,a.kt)("div",{className:"admonition admonition-important alert alert--info"},(0,a.kt)("div",{parentName:"div",className:"admonition-heading"},(0,a.kt)("h5",{parentName:"div"},(0,a.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,a.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,a.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"}))),"important")),(0,a.kt)("div",{parentName:"div",className:"admonition-content"},(0,a.kt)("p",{parentName:"div"},"Be sure to check out the ",(0,a.kt)("a",{parentName:"p",href:"https://www.nxp.com/document/guide/getting-started-with-the-nxp-edgeready-mcu-based-solution-for-3d-face-recognition:GS-SLN-VIZN3D-IOT"},"Getting Started Guide"),"\nfor an overview of the out-of-box features available in the SLN-VIZN3D-IOT Smart Lock application."))))}m.isMDXComponent=!0},642:function(e,t,n){t.Z=n.p+"assets/images/software_block_diagram-d920b2f35a02801e23fa050d5c213a3c.jpg"}}]);