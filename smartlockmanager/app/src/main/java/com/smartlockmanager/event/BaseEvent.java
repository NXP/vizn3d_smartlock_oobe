/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.event;

public class BaseEvent {

    public final long id;

    public BaseEvent() {
        id = System.nanoTime();
    }
}
