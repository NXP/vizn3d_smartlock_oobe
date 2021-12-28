---
sidebar_position: 1
title: Introduction
---

# Framework Introduction

This section will discuss the architecture design of the Smart Lock application.
The Smart Lock application is primarily designed around the use of a "framework" architecture
which is composed of several different parts.

These constituent parts include:

* Device Managers
* Hardware Abstraction Layer (HAL) devices
* Messages/Events

![Architecture Diagram](../../static/img/framework/framework_arch_diagram.jpg)

Each of these different components will be discussed in detail in the following sections.

## Design Goals

The architectural design of the Smart Lock application software was centered around 3 primary goals:

1. Ease-of-use
2. Flexibility/portability
3. Performance

In the course of a project's development,
many problems can arise which hinder the speed of that development.
The framework architecture was designed to help combat those problems.

The SLN-VIZN3D-IOT platform is designed with the goal of speeding up the time to market for vision and other machine-learning applications.
In order to ensure a speedy time to market,
it is critical that the software itself is easy to understand and easy to modify.
Keeping this goal in mind,
the architecture of the Smart Lock software was designed to be easy to modify without being restrictive,
and without coming at the cost of performance.

## Relevant Files

The files which pertain to the framework architecture
can primarily be found in the `source/`, `framework/`, and `HAL/` folders of the `sln_vizn3d_iot_smart_lock` application.
Because the Smart Lock application is designed around the use of the framework architecture,
it is likely that the bulk of a developer's efforts will be focused on the contents of these folders.
