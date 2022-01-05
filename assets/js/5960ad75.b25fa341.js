"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[761],{3905:function(e,n,t){t.d(n,{Zo:function(){return d},kt:function(){return v}});var a=t(7294);function r(e,n,t){return n in e?Object.defineProperty(e,n,{value:t,enumerable:!0,configurable:!0,writable:!0}):e[n]=t,e}function i(e,n){var t=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);n&&(a=a.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),t.push.apply(t,a)}return t}function o(e){for(var n=1;n<arguments.length;n++){var t=null!=arguments[n]?arguments[n]:{};n%2?i(Object(t),!0).forEach((function(n){r(e,n,t[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(t)):i(Object(t)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(t,n))}))}return e}function s(e,n){if(null==e)return{};var t,a,r=function(e,n){if(null==e)return{};var t,a,r={},i=Object.keys(e);for(a=0;a<i.length;a++)t=i[a],n.indexOf(t)>=0||(r[t]=e[t]);return r}(e,n);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(a=0;a<i.length;a++)t=i[a],n.indexOf(t)>=0||Object.prototype.propertyIsEnumerable.call(e,t)&&(r[t]=e[t])}return r}var c=a.createContext({}),l=function(e){var n=a.useContext(c),t=n;return e&&(t="function"==typeof e?e(n):o(o({},n),e)),t},d=function(e){var n=l(e.components);return a.createElement(c.Provider,{value:n},e.children)},m={inlineCode:"code",wrapper:function(e){var n=e.children;return a.createElement(a.Fragment,{},n)}},p=a.forwardRef((function(e,n){var t=e.components,r=e.mdxType,i=e.originalType,c=e.parentName,d=s(e,["components","mdxType","originalType","parentName"]),p=l(t),v=r,u=p["".concat(c,".").concat(v)]||p[v]||m[v]||i;return t?a.createElement(u,o(o({ref:n},d),{},{components:t})):a.createElement(u,o({ref:n},d))}));function v(e,n){var t=arguments,r=n&&n.mdxType;if("string"==typeof e||r){var i=t.length,o=new Array(i);o[0]=p;var s={};for(var c in n)hasOwnProperty.call(n,c)&&(s[c]=n[c]);s.originalType=e,s.mdxType="string"==typeof e?e:r,o[1]=s;for(var l=2;l<i;l++)o[l]=t[l];return a.createElement.apply(null,o)}return a.createElement.apply(null,t)}p.displayName="MDXCreateElement"},4961:function(e,n,t){t.r(n),t.d(n,{frontMatter:function(){return s},contentTitle:function(){return c},metadata:function(){return l},toc:function(){return d},default:function(){return p}});var a=t(7462),r=t(3366),i=(t(7294),t(3905)),o=["components"],s={sidebar_position:1},c="Overview",l={unversionedId:"framework/device-managers/overview",id:"framework/device-managers/overview",isDocsHomePage:!1,title:"Overview",description:"As the name would imply,",source:"@site/docs/framework/device-managers/overview.md",sourceDirName:"framework/device-managers",slug:"/framework/device-managers/overview",permalink:"/vizn3d_smartlock_oobe/docs/framework/device-managers/overview",editUrl:"https://github.com/nxp/vizn3d_smartlock_oobe/docs/docs/docs/framework/device-managers/overview.md",tags:[],version:"current",sidebarPosition:1,frontMatter:{sidebar_position:1},sidebar:"tutorialSidebar",previous:{title:"Introduction",permalink:"/vizn3d_smartlock_oobe/docs/framework/introduction"},next:{title:"Vision Input Manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device-managers/input_manager"}},d=[{value:"Initialization Flow",id:"initialization-flow",children:[],level:2}],m={toc:d};function p(e){var n=e.components,t=(0,r.Z)(e,o);return(0,i.kt)("wrapper",(0,a.Z)({},m,t,{components:n,mdxType:"MDXLayout"}),(0,i.kt)("h1",{id:"overview"},"Overview"),(0,i.kt)("p",null,'As the name would imply,\ndevice managers are responsible for "managing" devices used by the system.\nEach device type (input, output, etc.) has its own type-specific device manager.'),(0,i.kt)("p",null,"A device manager serves two primary purposes:"),(0,i.kt)("ul",null,(0,i.kt)("li",{parentName:"ul"},"Initializing and starting each device registered to that manager"),(0,i.kt)("li",{parentName:"ul"},"Sending data to and receiving data from each device registered to that manager")),(0,i.kt)("p",null,"This section will avoid low-level implementation details of the device managers\nand instead focus on the device manager APIs and the startup flow for the device managers.\nThe device managers themselves are provided as a library binary file to,\nin part,\nhelp abstract the underlying implementation details and encourage developers to focus on the HAL devices being managed instead."),(0,i.kt)("div",{className:"admonition admonition-info alert alert--info"},(0,i.kt)("div",{parentName:"div",className:"admonition-heading"},(0,i.kt)("h5",{parentName:"div"},(0,i.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,i.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,i.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"}))),"info")),(0,i.kt)("div",{parentName:"div",className:"admonition-content"},(0,i.kt)("p",{parentName:"div"},"The device managers themselves are provided as a library binary file in the ",(0,i.kt)("inlineCode",{parentName:"p"},"framework")," folder,\nwhile the APIs for each manager can be found in the ",(0,i.kt)("inlineCode",{parentName:"p"},"framework/inc")," folder."))),(0,i.kt)("h2",{id:"initialization-flow"},"Initialization Flow"),(0,i.kt)("p",null,"Before a device manager can properly manage devices, it must follow a specific startup process.\nThe startup process for device managers is summarized as follows:"),(0,i.kt)("ol",null,(0,i.kt)("li",{parentName:"ol"},"Initialize managers"),(0,i.kt)("li",{parentName:"ol"},"Register each device to their respective manager"),(0,i.kt)("li",{parentName:"ol"},"Start managers")),(0,i.kt)("p",null,"This process is clearly demonstrated in the ",(0,i.kt)("inlineCode",{parentName:"p"},"main")," function found in ",(0,i.kt)("inlineCode",{parentName:"p"},"source/main.cpp")),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="source/main.cpp" {9-16}',title:'"source/main.cpp"',"{9-16}":!0},'/*\n * @brief   Application entry point.\n */\nint main(void)\n{\n    /* Init board hardware. */\n    APP_BoardInit();\n    LOGD("[MAIN]:Started");\n    /* init the framework*/\n    APP_InitFramework();\n\n    /* register the hal devices*/\n    APP_RegisterHalDevices();\n\n    /* start the framework*/\n    APP_StartFramework();\n\n    // start\n    vTaskStartScheduler();\n\n    while (1)\n    {\n        LOGD("#");\n    }\n\n    return 0;\n}\n')),(0,i.kt)("p",null,"As part of a manager's ",(0,i.kt)("inlineCode",{parentName:"p"},"start")," routine,\nthe manager will call the ",(0,i.kt)("inlineCode",{parentName:"p"},"init")," and ",(0,i.kt)("inlineCode",{parentName:"p"},"start")," functions of each of its registered devices."),(0,i.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,i.kt)("div",{parentName:"div",className:"admonition-heading"},(0,i.kt)("h5",{parentName:"div"},(0,i.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,i.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,i.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,i.kt)("div",{parentName:"div",className:"admonition-content"},(0,i.kt)("p",{parentName:"div"},"In general,\ndevelopers should only be concerned with adding/removing devices from the ",(0,i.kt)("inlineCode",{parentName:"p"},"APP_RegisterHalDevices()")," function as the ",(0,i.kt)("inlineCode",{parentName:"p"},"Init")," and ",(0,i.kt)("inlineCode",{parentName:"p"},"Start")," functions for each manager is already called by default inside the ",(0,i.kt)("inlineCode",{parentName:"p"},"APP_InitFramework()")," and ",(0,i.kt)("inlineCode",{parentName:"p"},"APP_StartFramework()")," functions in ",(0,i.kt)("inlineCode",{parentName:"p"},"main()"),"."))))}p.isMDXComponent=!0}}]);