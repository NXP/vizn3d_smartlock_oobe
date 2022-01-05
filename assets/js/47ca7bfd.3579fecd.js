"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[325],{3905:function(e,n,a){a.d(n,{Zo:function(){return p},kt:function(){return m}});var r=a(7294);function t(e,n,a){return n in e?Object.defineProperty(e,n,{value:a,enumerable:!0,configurable:!0,writable:!0}):e[n]=a,e}function i(e,n){var a=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);n&&(r=r.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),a.push.apply(a,r)}return a}function s(e){for(var n=1;n<arguments.length;n++){var a=null!=arguments[n]?arguments[n]:{};n%2?i(Object(a),!0).forEach((function(n){t(e,n,a[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(a)):i(Object(a)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(a,n))}))}return e}function l(e,n){if(null==e)return{};var a,r,t=function(e,n){if(null==e)return{};var a,r,t={},i=Object.keys(e);for(r=0;r<i.length;r++)a=i[r],n.indexOf(a)>=0||(t[a]=e[a]);return t}(e,n);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(r=0;r<i.length;r++)a=i[r],n.indexOf(a)>=0||Object.prototype.propertyIsEnumerable.call(e,a)&&(t[a]=e[a])}return t}var o=r.createContext({}),c=function(e){var n=r.useContext(o),a=n;return e&&(a="function"==typeof e?e(n):s(s({},n),e)),a},p=function(e){var n=c(e.components);return r.createElement(o.Provider,{value:n},e.children)},d={inlineCode:"code",wrapper:function(e){var n=e.children;return r.createElement(r.Fragment,{},n)}},u=r.forwardRef((function(e,n){var a=e.components,t=e.mdxType,i=e.originalType,o=e.parentName,p=l(e,["components","mdxType","originalType","parentName"]),u=c(a),m=t,g=u["".concat(o,".").concat(m)]||u[m]||d[m]||i;return a?r.createElement(g,s(s({ref:n},p),{},{components:a})):r.createElement(g,s({ref:n},p))}));function m(e,n){var a=arguments,t=n&&n.mdxType;if("string"==typeof e||t){var i=a.length,s=new Array(i);s[0]=u;var l={};for(var o in n)hasOwnProperty.call(n,o)&&(l[o]=n[o]);l.originalType=e,l.mdxType="string"==typeof e?e:t,s[1]=l;for(var c=2;c<i;c++)s[c]=a[c];return r.createElement.apply(null,s)}return r.createElement.apply(null,a)}u.displayName="MDXCreateElement"},6343:function(e,n,a){a.r(n),a.d(n,{frontMatter:function(){return l},contentTitle:function(){return o},metadata:function(){return c},toc:function(){return p},default:function(){return u}});var r=a(7462),t=a(3366),i=(a(7294),a(3905)),s=["components"],l={sidebar_position:5},o="Display Manager",c={unversionedId:"framework/device_managers/display_manager",id:"framework/device_managers/display_manager",isDocsHomePage:!1,title:"Display Manager",description:"APIs",source:"@site/docs/framework/device_managers/display_manager.md",sourceDirName:"framework/device_managers",slug:"/framework/device_managers/display_manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device_managers/display_manager",tags:[],version:"current",sidebarPosition:5,frontMatter:{sidebar_position:5},sidebar:"tutorialSidebar",previous:{title:"Camera Manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device_managers/camera_manager"},next:{title:"Vision Algorithm Manager",permalink:"/vizn3d_smartlock_oobe/docs/framework/device_managers/vision_algo_manager"}},p=[{value:"APIs",id:"apis",children:[{value:"FWK_DisplayManager_Init",id:"fwk_displaymanager_init",children:[],level:3},{value:"FWK_DisplayManager_DeviceRegister",id:"fwk_displaymanager_deviceregister",children:[],level:3},{value:"FWK_DisplayManager_Start",id:"fwk_displaymanager_start",children:[],level:3},{value:"FWK_DisplayManager_Deinit",id:"fwk_displaymanager_deinit",children:[],level:3}],level:2}],d={toc:p};function u(e){var n=e.components,a=(0,t.Z)(e,s);return(0,i.kt)("wrapper",(0,r.Z)({},d,a,{components:n,mdxType:"MDXLayout"}),(0,i.kt)("h1",{id:"display-manager"},"Display Manager"),(0,i.kt)("h2",{id:"apis"},"APIs"),(0,i.kt)("h3",{id:"fwk_displaymanager_init"},"FWK_DisplayManager_Init"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Init internal structures for display manager.\n * @return int Return 0 if the init process was successful\n */\nint FWK_DisplayManager_Init();\n")),(0,i.kt)("h3",{id:"fwk_displaymanager_deviceregister"},"FWK_DisplayManager_DeviceRegister"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Register a display device. All display devices need to be registered before FWK_DisplayManager_Start is\n * called.\n * @param dev Pointer to a display device structure\n * @return int Return 0 if registration was successful\n */\nint FWK_DisplayManager_DeviceRegister(display_dev_t *dev);\n")),(0,i.kt)("h3",{id:"fwk_displaymanager_start"},"FWK_DisplayManager_Start"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Spawn Display manager task which will call init/start for all registered display devices. Will start the flow\n * to recive frames from the camera.\n * @return int Return 0 if starting was successful\n */\nint FWK_DisplayManager_Start();\n")),(0,i.kt)("h3",{id:"fwk_displaymanager_deinit"},"FWK_DisplayManager_Deinit"),(0,i.kt)("pre",null,(0,i.kt)("code",{parentName:"pre",className:"language-c"},"/**\n * @brief Init internal structures for display manager.\n * @return int Return 0 if the init process was successful\n */\nint FWK_DisplayManager_Deinit();\n")),(0,i.kt)("div",{className:"admonition admonition-warning alert alert--danger"},(0,i.kt)("div",{parentName:"div",className:"admonition-heading"},(0,i.kt)("h5",{parentName:"div"},(0,i.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,i.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"12",height:"16",viewBox:"0 0 12 16"},(0,i.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M5.05.31c.81 2.17.41 3.38-.52 4.31C3.55 5.67 1.98 6.45.9 7.98c-1.45 2.05-1.7 6.53 3.53 7.7-2.2-1.16-2.67-4.52-.3-6.61-.61 2.03.53 3.33 1.94 2.86 1.39-.47 2.3.53 2.27 1.67-.02.78-.31 1.44-1.13 1.81 3.42-.59 4.78-3.42 4.78-5.56 0-2.84-2.53-3.22-1.25-5.61-1.52.13-2.03 1.13-1.89 2.75.09 1.08-1.02 1.8-1.86 1.33-.67-.41-.66-1.19-.06-1.78C8.18 5.31 8.68 2.45 5.05.32L5.03.3l.02.01z"}))),"warning")),(0,i.kt)("div",{parentName:"div",className:"admonition-content"},(0,i.kt)("p",{parentName:"div"},"Calling this function is unnecessary in most applications and should be used with caution."))))}u.isMDXComponent=!0}}]);