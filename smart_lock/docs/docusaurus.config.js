// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer/themes/github');
const darkCodeTheme = require('prism-react-renderer/themes/dracula');

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'SLN-VIZN3D-IOT Developer Guide',
  tagline: 'MCU-Based Offline Face Recognition with 3D Liveness Capabilities',
  url: 'https://nxp.github.io',
  baseUrl: '/vizn3d_smartlock_oobe/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  organizationName: 'NXP', // Usually your GitHub org/user name.
  projectName: 'vizn3d_smartlock_oobe', // Usually your repo name.

  presets: [
    [
      '@docusaurus/preset-classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          // Please change this to your repo.
          editUrl: 'https://github.com/nxp/vizn3d_smartlock_oobe/docs/docs/',
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],

  plugins: [require.resolve("@cmfcmf/docusaurus-search-local")],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: 'SLN-VIZN3D-IOT',
        logo: {
          alt: 'NXP Logo',
          src: 'img/logo.svg',
        },
        items: [
          {
            type: 'doc',
            docId: 'intro',
            position: 'left',
            label: 'Software Developer Guide',
          },
        ],
      },
      footer: {
        style: 'dark',
        links: [
          {
            title: 'Docs',
            items: [
              {
                label: 'Software Developer Guide',
                to: '/docs/intro',
              },
            ],
          },
          {
            title: 'Community',
            items: [
              {
                label: 'NXP i.MX RT Community',
                href: 'https://community.nxp.com/t5/i-MX-RT/bd-p/imxrt',
              },
            ],
          },
          {
            title: 'More',
            items: [
              {
                label: 'SLN-VIZN3D-IOT Homepage',
                href: 'https://www.nxp.com/mcu-vision3d',
              },
              {
                label: 'GitHub',
                href: 'https://github.com/nxp/vizn3d_smartlock_oobe',
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()} NXP, Built with Docusaurus.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
      },
    }),
};

module.exports = config;
