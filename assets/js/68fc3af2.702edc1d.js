"use strict";(self.webpackChunkdocs=self.webpackChunkdocs||[]).push([[700],{3905:function(e,t,a){a.d(t,{Zo:function(){return d},kt:function(){return m}});var n=a(7294);function i(e,t,a){return t in e?Object.defineProperty(e,t,{value:a,enumerable:!0,configurable:!0,writable:!0}):e[t]=a,e}function r(e,t){var a=Object.keys(e);if(Object.getOwnPropertySymbols){var n=Object.getOwnPropertySymbols(e);t&&(n=n.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),a.push.apply(a,n)}return a}function l(e){for(var t=1;t<arguments.length;t++){var a=null!=arguments[t]?arguments[t]:{};t%2?r(Object(a),!0).forEach((function(t){i(e,t,a[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(a)):r(Object(a)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(a,t))}))}return e}function s(e,t){if(null==e)return{};var a,n,i=function(e,t){if(null==e)return{};var a,n,i={},r=Object.keys(e);for(n=0;n<r.length;n++)a=r[n],t.indexOf(a)>=0||(i[a]=e[a]);return i}(e,t);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);for(n=0;n<r.length;n++)a=r[n],t.indexOf(a)>=0||Object.prototype.propertyIsEnumerable.call(e,a)&&(i[a]=e[a])}return i}var o=n.createContext({}),p=function(e){var t=n.useContext(o),a=t;return e&&(a="function"==typeof e?e(t):l(l({},t),e)),a},d=function(e){var t=p(e.components);return n.createElement(o.Provider,{value:t},e.children)},c={inlineCode:"code",wrapper:function(e){var t=e.children;return n.createElement(n.Fragment,{},t)}},h=n.forwardRef((function(e,t){var a=e.components,i=e.mdxType,r=e.originalType,o=e.parentName,d=s(e,["components","mdxType","originalType","parentName"]),h=p(a),m=i,v=h["".concat(o,".").concat(m)]||h[m]||c[m]||r;return a?n.createElement(v,l(l({ref:t},d),{},{components:a})):n.createElement(v,l({ref:t},d))}));function m(e,t){var a=arguments,i=t&&t.mdxType;if("string"==typeof e||i){var r=a.length,l=new Array(r);l[0]=h;var s={};for(var o in t)hasOwnProperty.call(t,o)&&(s[o]=t[o]);s.originalType=e,s.mdxType="string"==typeof e?e:i,l[1]=s;for(var p=2;p<r;p++)l[p]=a[p];return n.createElement.apply(null,l)}return n.createElement.apply(null,a)}h.displayName="MDXCreateElement"},682:function(e,t,a){a.r(t),a.d(t,{frontMatter:function(){return s},contentTitle:function(){return o},metadata:function(){return p},toc:function(){return d},default:function(){return h}});var n=a(7462),i=a(3366),r=(a(7294),a(3905)),l=["components"],s={sidebar_position:5},o="Display Devices",p={unversionedId:"framework/hal_devices/display",id:"framework/hal_devices/display",isDocsHomePage:!1,title:"Display Devices",description:"The Display HAL device provides an abstraction to represent many different display panels which may have different controllers, resolutions, color formats, and event connection interfaces.",source:"@site/docs/framework/hal_devices/display.md",sourceDirName:"framework/hal_devices",slug:"/framework/hal_devices/display",permalink:"/vizn3d_smartlock_oobe/docs/framework/hal_devices/display",tags:[],version:"current",sidebarPosition:5,frontMatter:{sidebar_position:5},sidebar:"tutorialSidebar",previous:{title:"Camera Devices",permalink:"/vizn3d_smartlock_oobe/docs/framework/hal_devices/camera"},next:{title:"VAlgo Devices",permalink:"/vizn3d_smartlock_oobe/docs/framework/hal_devices/valgo"}},d=[{value:"Device Definition",id:"device-definition",children:[],level:2},{value:"Operators",id:"operators",children:[{value:"Init",id:"init",children:[],level:3},{value:"Deinit",id:"deinit",children:[],level:3},{value:"Start",id:"start",children:[],level:3},{value:"Blit",id:"blit",children:[],level:3},{value:"InputNotify",id:"inputnotify",children:[],level:3}],level:2},{value:"Capabilities",id:"capabilities",children:[{value:"height",id:"height",children:[],level:3},{value:"width",id:"width",children:[],level:3},{value:"pitch",id:"pitch",children:[],level:3},{value:"left",id:"left",children:[],level:3},{value:"top",id:"top",children:[],level:3},{value:"right",id:"right",children:[],level:3},{value:"bottom",id:"bottom",children:[],level:3},{value:"rotate",id:"rotate",children:[],level:3},{value:"format",id:"format",children:[],level:3},{value:"srcFormat",id:"srcformat",children:[],level:3},{value:"frameBuffer",id:"framebuffer",children:[],level:3},{value:"callback",id:"callback",children:[],level:3},{value:"param",id:"param",children:[],level:3}],level:2},{value:"Example",id:"example",children:[],level:2}],c={toc:d};function h(e){var t=e.components,a=(0,i.Z)(e,l);return(0,r.kt)("wrapper",(0,n.Z)({},c,a,{components:t,mdxType:"MDXLayout"}),(0,r.kt)("h1",{id:"display-devices"},"Display Devices"),(0,r.kt)("p",null,"The ",(0,r.kt)("inlineCode",{parentName:"p"},"Display")," HAL device provides an abstraction to represent many different display panels which may have different controllers, resolutions, color formats, and event connection interfaces."),(0,r.kt)("p",null,'For example, in the VIZN3D kit, the "rk024hh298" panel is connected via the eLCDIF interface\nand the rk055ahd091 panel is connected via the LCDIF v2 interface.'),(0,r.kt)("div",{className:"admonition admonition-info alert alert--info"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"}))),"info")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},"A display HAL devices represents a display panel + interface."),(0,r.kt)("p",{parentName:"div"},"For example, the ",(0,r.kt)("inlineCode",{parentName:"p"},"hal_display_lcdif_rk024hh298.c")," is the display HAL device driver for the rk024hh298 panel with eLCDIF interface."),(0,r.kt)("p",{parentName:"div"},"This means a separate device driver is required for the same display using different interfaces."))),(0,r.kt)("p",null,"As with other device types,\ndisplay devices are controlled via their manager.\nThe Display Manager is responsible for managing all registered display HAL devices,\nand invoking display device operators (",(0,r.kt)("inlineCode",{parentName:"p"},"init"),", ",(0,r.kt)("inlineCode",{parentName:"p"},"start"),", etc.) as necessary."),(0,r.kt)("h2",{id:"device-definition"},"Device Definition"),(0,r.kt)("p",null,"The HAL device definition for display devices can be found under ",(0,r.kt)("inlineCode",{parentName:"p"},"framework/hal_api/hal_display_dev.h")," and is reproduced below:"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="display_dev_t"',title:'"display_dev_t"'},"typedef struct _display_dev display_dev_t;\n/*! @brief Attributes of a display device. */\nstruct _display_dev\n{\n    /* unique id which is assigned by Display Manager during the registration */\n    int id;\n    /* name of the device */\n    char name[DEVICE_NAME_MAX_LENGTH];\n    /* operations */\n    const display_dev_operator_t *ops;\n    /* private capability */\n    display_dev_private_capability_t cap;\n};\n")),(0,r.kt)("p",null,"The ",(0,r.kt)("a",{parentName:"p",href:"#operators"},"operators")," associated with display HAL devices are as shown below:"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="Operators"',title:'"Operators"'},"/*! @brief Operation that needs to be implemented by a display device */\ntypedef struct _display_dev_operator\n{\n    /* initialize the dev */\n    hal_display_status_t (*init)(\n        display_dev_t *dev,\n        int width, int height,\n        display_dev_callback_t callback,\n        void *param);\n    /* deinitialize the dev */\n    hal_display_status_t (*deinit)(const display_dev_t *dev);\n    /* start the dev */\n    hal_display_status_t (*start)(const display_dev_t *dev);\n    /* blit a buffer to the dev */\n    hal_display_status_t (*blit)(const display_dev_t *dev,\n                                void *frame,\n                                int width,\n                                int height);\n    /* input notify */\n    hal_display_status_t (*inputNotify)(const display_dev_t *dev, void *data);\n} display_dev_operator_t;\n")),(0,r.kt)("p",null,"The ",(0,r.kt)("a",{parentName:"p",href:"#capabilities"},"capabilities")," associated with display HAL devices are as shown below:"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="capability"',title:'"capability"'},"/*! @brief Structure that characterize the display device. */\ntypedef struct _display_dev_private_capability\n{\n    /* buffer resolution */\n    int height;\n    int width;\n    int pitch;\n    /* active rect */\n    int left;\n    int top;\n    int right;\n    int bottom;\n    /* rotate degree */\n    cw_rotate_degree_t rotate;\n    /* pixel format */\n    pixel_format_t format;\n    /* the source pixel format of the requested frame */\n    pixel_format_t srcFormat;\n    void *frameBuffer;\n    /* callback */\n    display_dev_callback_t callback;\n    /* param for the callback */\n    void *param;\n} display_dev_private_capability_t;\n")),(0,r.kt)("h2",{id:"operators"},"Operators"),(0,r.kt)("p",null,'Operators are functions which "operate" on a HAL device itself.\nOperators are akin to "public methods" in object oriented-languages,\nand are used by the Display Manager to setup, start, etc. each of its registered display devices.'),(0,r.kt)("p",null,"For more information about operators, see ",(0,r.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/hal_devices/overview#Operators"},"Operators"),"."),(0,r.kt)("h3",{id:"init"},"Init"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"hal_display_status_t (*init)(display_dev_t *dev,\n                            int width,\n                            int height,\n                            display_dev_callback_t callback,\n                            void *param);\n")),(0,r.kt)("p",null,"Initialize the display device."),(0,r.kt)("p",null,(0,r.kt)("inlineCode",{parentName:"p"},"Init")," should initialize any hardware resources the display device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires."),(0,r.kt)("p",null,"The ",(0,r.kt)("a",{parentName:"p",href:"#callback"},"callback function")," to the device's manager is typically installed as part of the ",(0,r.kt)("inlineCode",{parentName:"p"},"Init")," function as well."),(0,r.kt)("p",null,"This operator will be called by the Display Manager when the Display Manager task first starts."),(0,r.kt)("h3",{id:"deinit"},"Deinit"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"hal_display_status_t (*deinit)(const display_dev_t *dev);\n")),(0,r.kt)("p",null,'"Deinitialize" the display device.'),(0,r.kt)("p",null,(0,r.kt)("inlineCode",{parentName:"p"},"DeInit")," should release any hardware resources the display device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires."),(0,r.kt)("p",null,"This operator will be called by the Display Manager when the Display Manager task ends",(0,r.kt)("sup",{parentName:"p",id:"fnref-1"},(0,r.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1")),"."),(0,r.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},(0,r.kt)("sup",{parentName:"p",id:"fnref-1"},(0,r.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1")),"The ",(0,r.kt)("inlineCode",{parentName:"p"},"DeInit")," function generally will not be called under normal operation."))),(0,r.kt)("h3",{id:"start"},"Start"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"hal_display_status_t (*start)(const display_dev_t *dev);\n")),(0,r.kt)("p",null,"Start the display device."),(0,r.kt)("p",null,"The ",(0,r.kt)("inlineCode",{parentName:"p"},"Start")," operator will be called in the initialization stage of the Display Manager's task after the call to the ",(0,r.kt)("inlineCode",{parentName:"p"},"Init")," operator.\nThe startup of the display sensor and interface should be implemented in this operator.\nThis includes, for example, starting the interface and enabling the IRQ of the DMA used by the interface."),(0,r.kt)("h3",{id:"blit"},"Blit"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"hal_display_status_t (*blit)(const display_dev_t *dev,\n                            void *frame,\n                            int width,\n                            int height);\n")),(0,r.kt)("p",null,'Sends a frame to the display panel and "blits" the frame with any additional required components (UI overlay, etc.).'),(0,r.kt)("p",null,(0,r.kt)("inlineCode",{parentName:"p"},"Blit")," is called by the Display Manager once a previously requested frame of the matching ",(0,r.kt)("a",{parentName:"p",href:"#srcformat"},"srcFormat")," has been sent by a camera device.\nThe sending of the frame from the Display Manager to the display panel should be take place in this operator."),(0,r.kt)("p",null,(0,r.kt)("inlineCode",{parentName:"p"},"kStatus_HAL_DisplaySuccess")," should be returned if the frame was successfully sent to the display panel.\nAfter calling this operator, the Display Manager will request a new frame."),(0,r.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},"If the ",(0,r.kt)("inlineCode",{parentName:"p"},"Blit")," operator is working in asynchronous mode, the hardware will continue sending the frame buffer even after the return of the ",(0,r.kt)("inlineCode",{parentName:"p"},"Blit")," function call.\nIn this case, ",(0,r.kt)("inlineCode",{parentName:"p"},"kStatus_HAL_DisplayNonBlocking")," should be returned instead,\nand the Display Manager will not issue a new display frame request after this ",(0,r.kt)("inlineCode",{parentName:"p"},"Blit")," call."),(0,r.kt)("p",{parentName:"div"},"To request a new frame, the device should invoke the Display Manager's callback using a ",(0,r.kt)("inlineCode",{parentName:"p"},"kDisplayEvent_RequestFrame")," event to notify the completion of the sending of the previous frame.\nOnce the Display Manager sees this new request, it will requesting a new frame."))),(0,r.kt)("h3",{id:"inputnotify"},"InputNotify"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"    hal_display_status_t (*inputNotify)(const display_dev_t *dev, void *data);\n")),(0,r.kt)("p",null,"Handle input events."),(0,r.kt)("p",null,"The ",(0,r.kt)("inlineCode",{parentName:"p"},"InputNotify")," operator is called by the Display Manager whenever a ",(0,r.kt)("inlineCode",{parentName:"p"},"kFWKMessageID_InputNotify")," message is received by and forwarded from the Display Manager's message queue."),(0,r.kt)("p",null,"For more information regarding events and event handling, see ",(0,r.kt)("a",{parentName:"p",href:"/vizn3d_smartlock_oobe/docs/framework/events/overview"},"Events"),"."),(0,r.kt)("h2",{id:"capabilities"},"Capabilities"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="display_dev_private_capability_t"',title:'"display_dev_private_capability_t"'},"/*! @brief Structure that characterizes the display device. */\ntypedef struct _display_dev_private_capability\n{\n    /* buffer resolution */\n    int height;\n    int width;\n    int pitch;\n    /* active rect */\n    int left;\n    int top;\n    int right;\n    int bottom;\n    /* rotate degree */\n    cw_rotate_degree_t rotate;\n    /* pixel format */\n    pixel_format_t format;\n    /* the source pixel format of the requested frame */\n    pixel_format_t srcFormat;\n    void *frameBuffer;\n    /* callback */\n    display_dev_callback_t callback;\n    /* param for the callback */\n    void *param;\n} display_dev_private_capability_t;\n")),(0,r.kt)("p",null,"The ",(0,r.kt)("inlineCode",{parentName:"p"},"capabilities")," struct is primarily used for storing a callback to communicate information from the device back to the Display Manager.\nThis callback function is typically installed via a device's ",(0,r.kt)("inlineCode",{parentName:"p"},"init")," operator."),(0,r.kt)("p",null,"Display devices also maintain information regarding the size of the display, pixel format, and other information pertinent to the display."),(0,r.kt)("h3",{id:"height"},"height"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int height;\n")),(0,r.kt)("p",null,"The height of the display buffer."),(0,r.kt)("h3",{id:"width"},"width"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int width;\n")),(0,r.kt)("p",null,"The width of the display buffer."),(0,r.kt)("h3",{id:"pitch"},"pitch"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int pitch;\n")),(0,r.kt)("p",null,"The total number of bytes in one row of the display buffer."),(0,r.kt)("h3",{id:"left"},"left"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int left;\n")),(0,r.kt)("p",null,"The left edge of the active area",(0,r.kt)("sup",{parentName:"p",id:"fnref-1"},(0,r.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1"))," in the display frame buffer."),(0,r.kt)("div",{className:"admonition admonition-info alert alert--info"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"}))),"info")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},(0,r.kt)("sup",{parentName:"p",id:"fnref-1"},(0,r.kt)("a",{parentName:"sup",href:"#fn-1",className:"footnote-ref"},"1")),"The active area indicates the area of the display frame buffer that will be utilized."))),(0,r.kt)("h3",{id:"top"},"top"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int top;\n")),(0,r.kt)("p",null,"The top edge of the active area in the display frame buffer."),(0,r.kt)("h3",{id:"right"},"right"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int right;\n")),(0,r.kt)("p",null,"The right edge of the active area in the display frame buffer."),(0,r.kt)("h3",{id:"bottom"},"bottom"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"int bottom;\n")),(0,r.kt)("p",null,"The bottom edge of the active area in the display frame buffer."),(0,r.kt)("h3",{id:"rotate"},"rotate"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"typedef enum _cw_rotate_degree\n{\n    kCWRotateDegree_0 = 0,\n    kCWRotateDegree_90,\n    kCWRotateDegree_180,\n    kCWRotateDegree_270\n} cw_rotate_degree_t;\n")),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"cw_rotate_degree_t rotate;\n")),(0,r.kt)("p",null,"The rotate degree of the display frame buffer."),(0,r.kt)("h3",{id:"format"},"format"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"typedef enum _pixel_format\n{\n    /* 2d frame format */\n    kPixelFormat_RGB,\n    kPixelFormat_RGB565,\n    kPixelFormat_BGR,\n    kPixelFormat_Gray888,\n    kPixelFormat_Gray888X,\n    kPixelFormat_Gray,\n    kPixelFormat_Gray16,\n    kPixelFormat_YUV1P444_RGB,   /* color display sensor */\n    kPixelFormat_YUV1P444_Gray,  /* ir display sensor */\n    kPixelFormat_UYVY1P422_RGB,  /* color display sensor */\n    kPixelFormat_UYVY1P422_Gray, /* ir display sensor */\n    kPixelFormat_VYUY1P422,\n\n    /* 3d frame format */\n    kPixelFormat_Depth16,\n    kPixelFormat_Depth8,\n\n    kPixelFormat_YUV420P,\n\n    kPixelFormat_Invalid\n} pixel_format_t;\n")),(0,r.kt)("p",null,"The format of the display frame buffer."),(0,r.kt)("h3",{id:"srcformat"},"srcFormat"),(0,r.kt)("p",null,"The source format of the requested display frame buffer."),(0,r.kt)("p",null,"Because there may be multiple display devices operating at a time,\nthe display will check the ",(0,r.kt)("inlineCode",{parentName:"p"},"srcFormat")," property of the frame to determine whether it is from the display device it is expecting.\nThis prevents the display from displaying a 3D depth image when the user expects an RGB image, for example."),(0,r.kt)("h3",{id:"framebuffer"},"frameBuffer"),(0,r.kt)("p",null,"Pointer to the display frame buffer."),(0,r.kt)("h3",{id:"callback"},"callback"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="display_dev_callback_t"',title:'"display_dev_callback_t"'},"/**\n * @brief callback function to notify Display Manager that an async event took place\n * @param dev Device structure of the display device calling this function\n * @param event id of the event that took place\n * @param param Parameters\n * @param fromISR True if this operation takes place in an irq, 0 otherwise\n * @return 0 if the operation was successfully\n */\ntypedef int (*display_dev_callback_t)(const display_dev_t *dev,\n                                    display_event_t event,\n                                    void *param,\n                                    uint8_t fromISR);\n")),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"display_dev_callback_t callback;\n")),(0,r.kt)("p",null,"Callback to the Display Manager.\nThe HAL device invokes this callback to notify the Display Manager of specific events."),(0,r.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},"Currently, only the ",(0,r.kt)("inlineCode",{parentName:"p"},"kDisplayEvent_RequestFrame")," event callback is implemented in the Display Manager."))),(0,r.kt)("p",null,"The Display Manager will provide this callback to the device when the ",(0,r.kt)("inlineCode",{parentName:"p"},"init")," operator is called.\nAs a result, the HAL device should make sure to store the callback in the ",(0,r.kt)("inlineCode",{parentName:"p"},"init")," operator's implementation."),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="Example Display Device Init" {10}',title:'"Example',Display:!0,Device:!0,'Init"':!0,"{10}":!0},"hal_display_status_t HAL_DisplayDev_ExampleDev_Init(\n    display_dev_t *dev, int width, int height, display_dev_callback_t callback, void *param)\n{\n    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;\n\n    /* PERFORM INIT FUNCTIONALITY HERE */\n\n    ...\n\n    /* Installing callback function from manager... */\n    dev->cap.callback    = callback;\n\n    return ret;\n}\n")),(0,r.kt)("p",null,"The HAL device invokes this callback to notify the Display Manager of specific events."),(0,r.kt)("h3",{id:"param"},"param"),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c"},"void *param;\n")),(0,r.kt)("p",null,"The parameter of the Display Manager callback.",(0,r.kt)("sup",{parentName:"p",id:"fnref-2"},(0,r.kt)("a",{parentName:"sup",href:"#fn-2",className:"footnote-ref"},"2"))),(0,r.kt)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.kt)("div",{parentName:"div",className:"admonition-heading"},(0,r.kt)("h5",{parentName:"div"},(0,r.kt)("span",{parentName:"h5",className:"admonition-icon"},(0,r.kt)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.kt)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.kt)("div",{parentName:"div",className:"admonition-content"},(0,r.kt)("p",{parentName:"div"},"The ",(0,r.kt)("inlineCode",{parentName:"p"},"param")," field is not currently used by the framework in any way."))),(0,r.kt)("h2",{id:"example"},"Example"),(0,r.kt)("p",null,"The SLN-VIZN3D-IOT Smart Lock project has several display devices implemented for use as-is or as reference for implementing new display devices.\nThe source files for these display HAL devices can be found under ",(0,r.kt)("inlineCode",{parentName:"p"},"HAL/common/"),"."),(0,r.kt)("p",null,'Below is an example of the "rk024hh298" display HAL device driver ',(0,r.kt)("inlineCode",{parentName:"p"},"HAL/common/hal_display_lcdif_rk024hh298.c"),"."),(0,r.kt)("pre",null,(0,r.kt)("code",{parentName:"pre",className:"language-c",metastring:'title="HAL/common/hal_display_lcdif_rk024hh298.c"',title:'"HAL/common/hal_display_lcdif_rk024hh298.c"'},"\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Init(display_dev_t *dev,\n                                                        int width,\n                                                        int height,\n                                                        display_dev_callback_t callback,\n                                                        void *param);\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Uninit(const display_dev_t *dev);\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Start(const display_dev_t *dev);\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Blit(const display_dev_t *dev,\n                                                        void *frame,\n                                                        int width,\n                                                        int height);\nstatic hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_InputNotify(const display_dev_t *receiver,\n                                                                    void *data);\n\n/* The operators of the rk024hh298 Display HAL Device */\nconst static display_dev_operator_t s_DisplayDev_LcdifOps = {\n    .init        = HAL_DisplayDev_LcdifRk024hh2_Init,\n    .deinit      = HAL_DisplayDev_LcdifRk024hh2_Uninit,\n    .start       = HAL_DisplayDev_LcdifRk024hh2_Start,\n    .blit        = HAL_DisplayDev_LcdifRk024hh2_Blit,\n    .inputNotify = HAL_DisplayDev_LcdifRk024hh2_InputNotify,\n};\n\n/* rk024hh298 Display HAL Device */\nstatic display_dev_t s_DisplayDev_Lcdif = {\n    .id   = 0,\n    .name = DISPLAY_NAME,\n    .ops  = &s_DisplayDev_LcdifOps,\n    .cap  = {\n        .width       = DISPLAY_WIDTH,\n        .height      = DISPLAY_HEIGHT,\n        .pitch       = DISPLAY_WIDTH * DISPLAY_BYTES_PER_PIXEL,\n        .left        = 0,\n        .top         = 0,\n        .right       = DISPLAY_WIDTH - 1,\n        .bottom      = DISPLAY_HEIGHT - 1,\n        .rotate      = kCWRotateDegree_0,\n        .format      = kPixelFormat_RGB565,\n        .srcFormat   = kPixelFormat_UYVY1P422_RGB,\n        .frameBuffer = NULL,\n        .callback    = NULL,\n        .param       = NULL\n        }\n    };\n\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Init(display_dev_t *dev,\n                                                        int width,\n                                                        int height,\n                                                        display_dev_callback_t callback,\n                                                        void *param)\n{\n    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;\n\n    /* init the capability */\n    dev->cap.width       = width;\n    dev->cap.height      = height;\n    dev->cap.frameBuffer = (void *)&s_FrameBuffers[1];\n\n    /* store the callback and param for late using */\n    dev->cap.callback    = callback;\n\n    /* init the low level display panel and interface */\n\n    return ret;\n}\n\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Uninit(const display_dev_t *dev)\n{\n    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;\n    /* Currently do nothing for the Deinit as we didn't support the runtime de-registraion of the device */\n    return ret;\n}\n\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Start(const display_dev_t *dev)\n{\n    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;\n\n    /* start the display pannel and the interface */\n\n    return ret;\n}\n\nhal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Blit(const display_dev_t *dev, void *frame, int width, int height)\n{\n    hal_display_status_t ret = kStatus_HAL_DisplayNonBlocking;\n\n    /* blit the frame to the real display pannel */\n\n    return ret;\n}\n\nstatic hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_InputNotify(const display_dev_t *receiver, void *data)\n{\n    hal_display_status_t error           = kStatus_HAL_DisplaySuccess;\n    event_base_t eventBase               = *(event_base_t *)data;\n    event_status_t event_response_status = kEventStatus_Ok;\n\n    /* handle the events which are interested in */\n    if (eventBase.eventId == kEventID_SetDisplayOutputSource)\n    {\n\n    }\n\n    return error;\n}\n")))}h.isMDXComponent=!0}}]);