/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.model;

public abstract class Task implements Runnable {

    public static long sInternalId = 0;

    public final long id;

    public Task() {
        this.id = ++sInternalId;
    }
}
