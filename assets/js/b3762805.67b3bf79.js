"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[664],{3905:function(e,n,r){r.d(n,{Zo:function(){return g},kt:function(){return u}});var i=r(7294);function a(e,n,r){return n in e?Object.defineProperty(e,n,{value:r,enumerable:!0,configurable:!0,writable:!0}):e[n]=r,e}function t(e,n){var r=Object.keys(e);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);n&&(i=i.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),r.push.apply(r,i)}return r}function o(e){for(var n=1;n<arguments.length;n++){var r=null!=arguments[n]?arguments[n]:{};n%2?t(Object(r),!0).forEach((function(n){a(e,n,r[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(r)):t(Object(r)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(r,n))}))}return e}function s(e,n){if(null==e)return{};var r,i,a=function(e,n){if(null==e)return{};var r,i,a={},t=Object.keys(e);for(i=0;i<t.length;i++)r=t[i],n.indexOf(r)>=0||(a[r]=e[r]);return a}(e,n);if(Object.getOwnPropertySymbols){var t=Object.getOwnPropertySymbols(e);for(i=0;i<t.length;i++)r=t[i],n.indexOf(r)>=0||Object.prototype.propertyIsEnumerable.call(e,r)&&(a[r]=e[r])}return a}var l=i.createContext({}),c=function(e){var n=i.useContext(l),r=n;return e&&(r="function"==typeof e?e(n):o(o({},n),e)),r},g=function(e){var n=c(e.components);return i.createElement(l.Provider,{value:n},e.children)},m={inlineCode:"code",wrapper:function(e){var n=e.children;return i.createElement(i.Fragment,{},n)}},d=i.forwardRef((function(e,n){var r=e.components,a=e.mdxType,t=e.originalType,l=e.parentName,g=s(e,["components","mdxType","originalType","parentName"]),d=c(r),u=a,p=d["".concat(l,".").concat(u)]||d[u]||m[u]||t;return r?i.createElement(p,o(o({ref:n},g),{},{components:r})):i.createElement(p,o({ref:n},g))}));function u(e,n){var r=arguments,a=n&&n.mdxType;if("string"==typeof e||a){var t=r.length,o=new Array(t);o[0]=d;var s={};for(var l in n)hasOwnProperty.call(n,l)&&(s[l]=n[l]);s.originalType=e,s.mdxType="string"==typeof e?e:a,o[1]=s;for(var c=2;c<t;c++)o[c]=r[c];return i.createElement.apply(null,o)}return i.createElement.apply(null,r)}d.displayName="MDXCreateElement"},6356:function(e,n,r){r.r(n),r.d(n,{frontMatter:function(){return s},contentTitle:function(){return l},metadata:function(){return c},toc:function(){return g},default:function(){return d}});var i=r(7462),a=r(3366),t=(r(7294),r(3905)),o=["components"],s={sidebar_position:6},l="Vision Algorithm Manager",c={unversionedId:"framework/device-managers/vision_algo_manager",id:"framework/device-managers/vision_algo_manager",isDocsHomePage:!1,title:"Vision Algorithm Manager",description:"APIs",source:"@site/docs/framework/device-managers/vision_algo_manager.md",sourceDirName:"framework/device-managers",slug:"/framework/device-managers/vision_algo_manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device-managers/vision_algo_manager",editUrl:"https://github.com/nxp/vizn3d_smartlock_oobe/docs/docs/docs/framework/device-managers/vision_algo_manager.md",tags:[],version:"current",sidebarPosition:6,frontMatter:{sidebar_position:6},sidebar:"tutorialSidebar",previous:{title:"Display Manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device-managers/display_manager"},next:{title:"Voice Algorithm Manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device-managers/voice_algo_manager"}},g=[{value:"APIs",id:"apis",children:[{value:"FWK_VisionAlgoManager_Init",id:"fwk_visionalgomanager_init",children:[],level:3},{value:"FWK_VisionAlgoManager_DeviceRegister",id:"fwk_visionalgomanager_deviceregister",children:[],level:3},{value:"FWK_VisionAlgoManager_Start",id:"fwk_visionalgomanager_start",children:[],level:3},{value:"FWK_VisionAlgoManager_Deinit",id:"fwk_visionalgomanager_deinit",children:[],level:3}],level:2}],m={toc:g};function d(e){var n=e.components,r=(0,a.Z)(e,o);return(0,t.kt)("wrapper",(0,i.Z)({},m,r,{components:n,mdxType:"MDXLayout"}),(0,t.kt)("h1",{id:"vision-algorithm-manager"},"Vision Algorithm Manager"),(0,t.kt)("h2",{id:"apis"},"APIs"),(0,t.kt)("h3",{id:"fwk_visionalgomanager_init"},"FWK_VisionAlgoManager_Init"),(0,t.kt)("pre",null,(0,t.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Init internal structures for VisionAlgo manager.\n * @return int Return 0 if the init process was successful\n */\nint FWK_VisionAlgoManager_Init();\n")),(0,t.kt)("h3",{id:"fwk_visionalgomanager_deviceregister"},"FWK_VisionAlgoManager_DeviceRegister"),(0,t.kt)("pre",null,(0,t.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Register a vision algorithm device. All algorithm devices need to be registered before\n * FWK_VisionAlgoManager_Start is called\n * @param dev Pointer to a vision algo device structure\n * @return int Return 0 if registration was successful\n */\nint FWK_VisionAlgoManager_DeviceRegister(vision_algo_dev_t *dev);\n")),(0,t.kt)("h3",{id:"fwk_visionalgomanager_start"},"FWK_VisionAlgoManager_Start"),(0,t.kt)("pre",null,(0,t.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Spawn VisionAlgo manager task which will call init/start for all registered VisionAlgo devices\n * @return int Return 0 if the starting process was successul\n */\nint FWK_VisionAlgoManager_Start();\n")),(0,t.kt)("h3",{id:"fwk_visionalgomanager_deinit"},"FWK_VisionAlgoManager_Deinit"),(0,t.kt)("pre",null,(0,t.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Deinit VisionAlgoManager\n * @return int Return 0 if the deinit process was successful\n */\nint FWK_VisionAlgoManager_Deinit();\n")),(0,t.kt)("div",{className:"admonition admonition-warning alert alert--danger"},(0,t.kt)("div",{parentName:"div",className:"admonition-heading"},(0,t.kt)("h5",{parentName:"div"},(0,t.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,t.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"12",height:"16",viewBox:"0 0 12 16"},(0,t.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M5.05.31c.81 2.17.41 3.38-.52 4.31C3.55 5.67 1.98 6.45.9 7.98c-1.45 2.05-1.7 6.53 3.53 7.7-2.2-1.16-2.67-4.52-.3-6.61-.61 2.03.53 3.33 1.94 2.86 1.39-.47 2.3.53 2.27 1.67-.02.78-.31 1.44-1.13 1.81 3.42-.59 4.78-3.42 4.78-5.56 0-2.84-2.53-3.22-1.25-5.61-1.52.13-2.03 1.13-1.89 2.75.09 1.08-1.02 1.8-1.86 1.33-.67-.41-.66-1.19-.06-1.78C8.18 5.31 8.68 2.45 5.05.32L5.03.3l.02.01z"}))),"warning")),(0,t.kt)("div",{parentName:"div",className:"admonition-content"},(0,t.kt)("p",{parentName:"div"},"Calling this function is unnecessary in most applications and should be used with caution."))))}d.isMDXComponent=!0}}]);