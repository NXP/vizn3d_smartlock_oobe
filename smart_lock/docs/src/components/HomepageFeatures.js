import React from 'react';
import clsx from 'clsx';
import styles from './HomepageFeatures.module.css';

const FeatureList = [
  {
    title: 'Overview',
    Svg: require('../../static/img/VIZN3D-VALUE-PROPOSITION.svg').default,
    description: (
      <>
        NXP’s EdgeReady MCU-based solution for 3D face recognition leverages the i.MX RT117F crossover MCU, enabling developers to quickly and easily add 3D face recognition with advanced liveness detection to their products, with the confidence that it will work in even the most challenging outdoor lighting conditions and can resist the most sophisticated spoofing attacks. 3D liveness detection defeats spoofing attempts using photographs or 3D models, and uses a high performance 3D structured light camera module (SLM) with an optional low-cost CMOS sensor-based RGB camera, without requiring the use an expensive, power hungry, Linux-based MPU with long boot times. The development kit for this solution, SLN-VIZN3D-IOT, comes with fully integrated turnkey software, for quick out-of-the-box operation, minimizing time to market, risk and development effort. Face recognition and liveness detection are done entirely offline, on the i.MX RT117F MCU, without using the cloud, addressing the privacy concerns of some consumers and eliminating the latency associated with cloud-based implementations. The solution includes an available remote registration capability to allow end users to register their faces from mobile devices.
      </>
    ),
  },
];

function Feature({Svg, title, description}) {
  return (
    <div className={clsx('col')}>
      <div className="text--center">
        <Svg className={styles.featureSvg} alt={title} />
      </div>
      <div className="text--left padding-horiz--md">
        <h3>{title}</h3>
      </div>
      <div className="text--left padding-horiz--md">
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
