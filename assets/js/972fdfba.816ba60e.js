"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[15],{3905:function(e,t,n){n.d(t,{Zo:function(){return s},kt:function(){return m}});var a=n(7294);function i(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function o(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);t&&(a=a.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,a)}return n}function r(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?o(Object(n),!0).forEach((function(t){i(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):o(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function l(e,t){if(null==e)return{};var n,a,i=function(e,t){if(null==e)return{};var n,a,i={},o=Object.keys(e);for(a=0;a<o.length;a++)n=o[a],t.indexOf(n)>=0||(i[n]=e[n]);return i}(e,t);if(Object.getOwnPropertySymbols){var o=Object.getOwnPropertySymbols(e);for(a=0;a<o.length;a++)n=o[a],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(i[n]=e[n])}return i}var p=a.createContext({}),d=function(e){var t=a.useContext(p),n=t;return e&&(n="function"==typeof e?e(t):r(r({},t),e)),n},s=function(e){var t=d(e.components);return a.createElement(p.Provider,{value:t},e.children)},c={inlineCode:"code",wrapper:function(e){var t=e.children;return a.createElement(a.Fragment,{},t)}},u=a.forwardRef((function(e,t){var n=e.components,i=e.mdxType,o=e.originalType,p=e.parentName,s=l(e,["components","mdxType","originalType","parentName"]),u=d(n),m=i,h=u["".concat(p,".").concat(m)]||u[m]||c[m]||o;return n?a.createElement(h,r(r({ref:t},s),{},{components:n})):a.createElement(h,r({ref:t},s))}));function m(e,t){var n=arguments,i=t&&t.mdxType;if("string"==typeof e||i){var o=n.length,r=new Array(o);r[0]=u;var l={};for(var p in t)hasOwnProperty.call(t,p)&&(l[p]=t[p]);l.originalType=e,l.mdxType="string"==typeof e?e:i,r[1]=l;for(var d=2;d<o;d++)r[d]=n[d];return a.createElement.apply(null,r)}return a.createElement.apply(null,n)}u.displayName="MDXCreateElement"},4157:function(e,t,n){n.r(t),n.d(t,{frontMatter:function(){return l},contentTitle:function(){return p},metadata:function(){return d},toc:function(){return s},default:function(){return u}});var a=n(7462),i=n(3366),o=(n(7294),n(3905)),r=["components"],l={sidebar_position:1},p="Introduction",d={unversionedId:"bootloader/introduction",id:"bootloader/introduction",isDocsHomePage:!1,title:"Introduction",description:'The Smart Lock project uses a "bootloader + main application" architecture to provide additional security and update-related functionality to the main application.',source:"@site/docs/bootloader/introduction.md",sourceDirName:"bootloader",slug:"/bootloader/introduction",permalink:"/vizn3d_smartlock_oobe/docs/bootloader/introduction",editUrl:"https://github.com/nxp/vizn3d_smartlock_oobe/docs/docs/docs/bootloader/introduction.md",tags:[],version:"current",sidebarPosition:1,frontMatter:{sidebar_position:1},sidebar:"tutorialSidebar",previous:{title:"Setup and Installation",permalink:"/vizn3d_smartlock_oobe/docs/setup-and-install"},next:{title:"Overview",permalink:"/vizn3d_smartlock_oobe/docs/bootloader/boot-modes/overview"}},s=[{value:"Why use a bootloader?",id:"why-use-a-bootloader",children:[],level:2},{value:"Application Banks",id:"application-banks",children:[],level:2},{value:"Logging",id:"logging",children:[],level:2}],c={toc:s};function u(e){var t=e.components,n=(0,i.Z)(e,r);return(0,o.kt)("wrapper",(0,a.Z)({},c,n,{components:t,mdxType:"MDXLayout"}),(0,o.kt)("h1",{id:"introduction"},"Introduction"),(0,o.kt)("p",null,'The Smart Lock project uses a "bootloader + main application" architecture to provide additional security and update-related functionality to the main application.\nThe bootloader handles all boot-related tasks including, but not limited to:'),(0,o.kt)("ul",null,(0,o.kt)("li",{parentName:"ul"},"Launching the main application and, if necessary, initializing peripherals"),(0,o.kt)("li",{parentName:"ul"},"Firmware updates using either the Mass Storage Device (MSD), Over-the-Air, or Over-the-Wire update method",(0,o.kt)("ul",{parentName:"li"},(0,o.kt)("li",{parentName:"ul"},'Protects against update failures by using a primary and backup application "flash bank"'))),(0,o.kt)("li",{parentName:"ul"},"Image certification/verification",(0,o.kt)("sup",{parentName:"li",id:"fnref-1"},(0,o.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1")))),(0,o.kt)("p",null,(0,o.kt)("sup",{parentName:"p",id:"fnref-1"},(0,o.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1"))," The SLN-VIZN3D-IOT does not currently support any bootloader security features."),(0,o.kt)("h2",{id:"why-use-a-bootloader"},"Why use a bootloader?"),(0,o.kt)("p",null,"By separating the boot process from the main application,\nthe main application can be safely updated and verified without the risk of creating an irrecoverable state due to a failed update, or running a malicious,\nunauthorized and unsigned firmware binary flashed by a bad actor.\nIt is essential in any production application that precautions be taken to ensure the integrity and stability of the firmware before, during, and after an update,\nand the bootloader application is simply one measure to help provide this assurance."),(0,o.kt)("p",null,"The following sections will describe how to use many of the bootloader's primary features in order to assist developer's interested in understanding, utilizing, and expanding them."),(0,o.kt)("h2",{id:"application-banks"},"Application Banks"),(0,o.kt)("p",null,'The bootloader file system uses dual application "banks" referred to as "Bank A" and "Bank B" to provide a backup/redundancy "known good" application to prevent bricking when flashing an update via either the MSD, OTA, or OTW update method.\nFor example,\nif an application update is being flashed via MSD to the Bank A application bank, even if that update should fail midway through Bank B will still contain a fully operational backup.'),(0,o.kt)("p",null,"In the SLN-VIZN3D-IOT, Bank A is located at ",(0,o.kt)("inlineCode",{parentName:"p"},"0x30200000")," while Bank B is located at ",(0,o.kt)("inlineCode",{parentName:"p"},"0x30800000"),".\nSpecifying an application's flash address can be done prior to compilation of the application via the ",(0,o.kt)("inlineCode",{parentName:"p"},"Properties->MCU Settings")," menu as shown in the figure below:"),(0,o.kt)("p",null,"Providing an application binary built for the proper application bank address is crucial during MSD, OTA, and OTW updates, and failure to do so will result in a failure to flash the binary."),(0,o.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,o.kt)("div",{parentName:"div",className:"admonition-heading"},(0,o.kt)("h5",{parentName:"div"},(0,o.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,o.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,o.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,o.kt)("div",{parentName:"div",className:"admonition-content"},(0,o.kt)("p",{parentName:"div"},"The bootloader will not automatically recover from a botched flashing procedure,\nbut will instead revert to the alternate working application flash bank instead."))),(0,o.kt)("h2",{id:"logging"},"Logging"),(0,o.kt)("p",null,"The bootloader supports debug logging over UART to help diagnose and debug issues that may arise while using or modifying the bootloader.\nFor example,\nthe debug logger can be helpful when trying to understand why an application update might have failed."),(0,o.kt)("p",null,"Logging is enabled by default in the ",(0,o.kt)("inlineCode",{parentName:"p"},"Debug")," build mode configuration.\nThe logging functionality, however, comes with an increase in bootloader performance, and can slow down the boot process by as much as 200ms.\nAs a result,\nit may be desirable to disable debug logging in production applications.\nTo disable logging in the bootloader,\nsimply build and run the bootloader in the ",(0,o.kt)("inlineCode",{parentName:"p"},"Release")," build mode configuration.\nThis can be done by right-clicking on the bootloader project in the ",(0,o.kt)("inlineCode",{parentName:"p"},"Project Explorer")," view\nand navigating to ",(0,o.kt)("inlineCode",{parentName:"p"},"Build Configurations -> Set Active -> Release")," as shown in the figure below:"),(0,o.kt)("p",null,"To make use of the debug logging feature,\nuse a UART->USB converter to:"),(0,o.kt)("ul",null,(0,o.kt)("li",{parentName:"ul"},"Connect ",(0,o.kt)("inlineCode",{parentName:"li"},"GND")," pin of converter to ",(0,o.kt)("inlineCode",{parentName:"li"},"J202: Pin 8")),(0,o.kt)("li",{parentName:"ul"},"Connect ",(0,o.kt)("inlineCode",{parentName:"li"},"TX"),"  pin of converter to ",(0,o.kt)("inlineCode",{parentName:"li"},"J202: Pin 3")),(0,o.kt)("li",{parentName:"ul"},"Connect ",(0,o.kt)("inlineCode",{parentName:"li"},"RX"),"  pin of converter to ",(0,o.kt)("inlineCode",{parentName:"li"},"J202: Pin 4"))),(0,o.kt)("p",null,"Once the converter has been properly attached,\nconnect to the board using a serial terminal emulator like ",(0,o.kt)("em",{parentName:"p"},"PuTTY")," or ",(0,o.kt)("em",{parentName:"p"},"Tera Term")," configured with the following serial settings:"),(0,o.kt)("ul",null,(0,o.kt)("li",{parentName:"ul"},"Speed: ",(0,o.kt)("inlineCode",{parentName:"li"},"115200")),(0,o.kt)("li",{parentName:"ul"},"Data: ",(0,o.kt)("inlineCode",{parentName:"li"},"8 Bit")),(0,o.kt)("li",{parentName:"ul"},"Parity: ",(0,o.kt)("inlineCode",{parentName:"li"},"None")),(0,o.kt)("li",{parentName:"ul"},"Stop Bits: ",(0,o.kt)("inlineCode",{parentName:"li"},"1 bit")),(0,o.kt)("li",{parentName:"ul"},"Flow Control: ",(0,o.kt)("inlineCode",{parentName:"li"},"None"))))}u.isMDXComponent=!0}}]);