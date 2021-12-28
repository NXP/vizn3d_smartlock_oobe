/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

package com.smartlockmanager.database;

import android.graphics.Bitmap;

import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity
public class User {
    @PrimaryKey()
    public int uid;

    @ColumnInfo(name = "name")
    public String name;

    @ColumnInfo(name = "photo", typeAffinity = ColumnInfo.BLOB)
    public byte[] photo;

    public User(int uid, String name, byte[] photo){
        this.uid = uid;
        this.name = name;
        this.photo = photo;
    }

    public String getName(){
        return name;
    }

    public int getID(){
        return uid;
    }
}