/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

package com.smartlockmanager.activity;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.content.ContextCompat;

import com.smartlockmanager.R;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.AutoFitSurfaceView;
import com.smartlockmanager.utility.AutoFitTextureView;
import com.smartlockmanager.utility.Algorithm;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Base64;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.TextureView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import static com.smartlockmanager.utility.SdkUtils.fullScreen;

public class RegistrationActivity extends AppCompatActivity {

    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_RA";

    /**
     * Request code for skipping activity on back
     */
    private static final int REQUEST_CODE = 100;

    /**
     * Camera state: Showing camera preview.
     */
    private static final int STATE_PREVIEW = 0;

    /**
     * Camera state: Waiting for the focus to be locked.
     */
    private static final int STATE_WAITING_LOCK = 1;

    /**
     * Camera state: Waiting for the exposure to be precapture state.
     */
    private static final int STATE_WAITING_PRECAPTURE = 2;

    /**
     * Camera state: Waiting for the exposure state to be something other than precapture.
     */
    private static final int STATE_WAITING_NON_PRECAPTURE = 3;

    /**
     * Camera state: Picture was taken.
     */
    private static final int STATE_PICTURE_TAKEN = 4;

    /**
     * Camera state: Showing camera preview.
     */
    private static final int STATE_START_REGISTRATION = 5;

    /**
     * Camera state: Showing camera preview.
     */
    private static final int STATE_FINISH_REGISTRATION = 6;

    private static final int STATE_TEST = 7;

    /**
     * Max preview width that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_WIDTH = 1920;

    /**
     * Max preview height that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_HEIGHT = 1080;

    /**
     * Offset used for cropping the bitmap
     */
    private static final int BMP_OFFSET = 64;

    /**
     * {@link TextureView.SurfaceTextureListener} handles several lifecycle events on a
     * {@link TextureView}.
     */
    private final TextureView.SurfaceTextureListener mSurfaceTextureListener
            = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
            Log.d(TAG, "+onSurfaceTextureAvailable:WXH:" + width + "X" + height);
            openCamera(width, height);
            //openCamera(selectSize.getWidth(), selectSize.getHeight());
            Log.d(TAG, "-onSurfaceTextureAvailable");
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
            configureTransform(width, height);
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture texture) {
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture texture) {
        }

    };

    /**
     * ID of the current {@link CameraDevice}.
     */
    private String mCameraId;

    /**
     * Store the current orientation.
     */
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    /**
     * An {@link AutoFitTextureView} for camera preview.
     */
    private AutoFitTextureView mTextureViewCamera;

    private AutoFitSurfaceView mSurfaceViewOverlay;
    private SurfaceHolder mSurfaceHolderOverlay;

    /**
     * A {@link CameraCaptureSession } for camera preview.
     */
    private CameraCaptureSession mCaptureSession;

    /**
     * A reference to the opened {@link CameraDevice}.
     */
    private CameraDevice mCameraDevice;

    /**
     * The {@link android.util.Size} of camera preview.
     */
    private Size mPreviewSize;

    /**
     * {@link CameraDevice.StateCallback} is called when {@link CameraDevice} changes its state.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
            finish();
        }
    };

    /**
     * An additional thread for running tasks that shouldn't block the UI.
     */
    private HandlerThread mBackgroundThread;

    /**
     * A {@link Handler} for running tasks in the background.
     */
    private Handler mBackgroundHandler;

    /**
     * An {@link ImageReader} that handles still image capture.
     */
    private ImageReader mImageReader;

    /**
     * This is the output file for our picture.
     */
    private File mFile;

    /**
     * This a callback object for the {@link ImageReader}. "onImageAvailable" will be called when a
     * still image is ready to be saved.
     */
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener
            = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
            mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
        }

    };

    /**
     * {@link CaptureRequest.Builder} for the camera preview
     */
    private CaptureRequest.Builder mPreviewRequestBuilder;

    /**
     * {@link CaptureRequest} generated by {@link #mPreviewRequestBuilder}
     */
    private CaptureRequest mPreviewRequest;

    /**
     * The current state of camera state for taking pictures.
     *
     * @see #mCaptureCallback
     */
    private int mState = STATE_PREVIEW;

    private int mPrevState = STATE_PREVIEW;

    private static final int REGISTRATION_NOT_START = 0;
    private static final int REGISTRATION_START = 1;
    
    private int mRegistrationState = REGISTRATION_NOT_START;
    
    private Intent pendingIntent;

    /**
     * A {@link Semaphore} to prevent the app from exiting before closing the camera.
     */
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);

    /**
     * Whether the current camera device supports Flash or not.
     */
    private boolean mFlashSupported;

    /**
     * Orientation of the camera sensor
     */
    private int mSensorOrientation;

    public byte[] getPixelsRGB(Bitmap image) {
       // calculate how many bytes our image consists of
       int bytes = image.getByteCount();
       ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
       image.copyPixelsToBuffer(buffer); // Move the byte data to the buffer
       byte[] temp = buffer.array(); // Get the underlying array containing the
       int rgbBytes = bytes / 4 * 3;
       // Log.e(TAG, "bytes" + bytes + "rgb" + rgbBytes);
       byte[] RGB = new byte[rgbBytes];
       for (int i = 0; i < bytes / 4; i++) {
           RGB[i * 3] = temp[i * 4];
           RGB[i * 3 + 1] = temp[i * 4 + 1];
           RGB[i * 3 + 2] = temp[i * 4 + 2];
           /*
            * if (i < 16) { Log.e(TAG, "[" + byteTo16(temp[i*4]) + "," +
            * byteTo16(temp[i*4+1]) + "," + byteTo16(temp[i*4+2]) + "," +
            * byteTo16(temp[i*4+3])+ "]"); }
            */
       }
       return RGB;
    }

    public Bitmap rgbToGrayscale(Bitmap bm){
       int w = bm.getWidth();
       int h = bm.getHeight();
       Bitmap gray = Bitmap.createBitmap(w, h, bm.getConfig());
       Canvas canvas = new Canvas(gray);
       Paint paint = new Paint();
       ColorMatrix cm = new ColorMatrix();
       cm.setSaturation(0);
       ColorMatrixColorFilter filter = new ColorMatrixColorFilter(cm);
       paint.setColorFilter(filter);
       canvas.drawBitmap(bm, 0,0, paint);

       return gray;
    }

    /**
     * A {@link CameraCaptureSession.CaptureCallback} that handles events related to JPEG capture.
     */
    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        private void process_registration() {
            int width = mTextureViewCamera.getWidth();
            int height = mTextureViewCamera.getHeight();

            Bitmap cameraBmp = mTextureViewCamera.getBitmap();

            if (width > ALGORITHM_SUPPORT_MAX_WIDTH || height > ALGORITHM_SUPPORT_MAX_HEIGHT) {
                float scaleWidth = ((float) ALGORITHM_SUPPORT_MAX_WIDTH) / width;
                float scaleHeight = ((float) ALGORITHM_SUPPORT_MAX_HEIGHT) / height;
                mScale = Math.min(scaleHeight, scaleWidth);
                Matrix matrix = new Matrix();
                matrix.setScale(mScale, mScale);
                cameraBmp = Bitmap.createBitmap(cameraBmp, 0,0,width, height, matrix, false);
                width  = cameraBmp.getWidth();
                height = cameraBmp.getHeight();
                //Log.e(TAG, "scale:wxh" + width + "x" + height);
            }

            Log.d(TAG, "mTextureViewCamera:" +"[" + width + "," + height + "]");

            final Bitmap grayBitmap = rgbToGrayscale(cameraBmp);
            byte[] FrameData = getPixelsRGB(grayBitmap);
            mFaceBox = new int[]{-1, -1, -1, -1};
            int ret = mAlgorithm.Registration(FrameData, width, height, mFaceBox, mFaceFeature);
            //Log.d(TAG, "Registration:" + ret + ", FaceBox:[" + mFaceBox[0] + "," + mFaceBox[1] + "," + mFaceBox[2] + "," + mFaceBox[3] + "]");

            // draw recognition face box
            if(mFaceBox[0] != -1 ) {
                Paint paint = new Paint();
                if(ret == 0){
                    paint.setColor(Color.GREEN);
                } else {
                    paint.setColor(Color.RED);
                }

                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth(4f);

                Canvas canvas = mSurfaceHolderOverlay.lockCanvas();
                canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
                Rect rect = new Rect((int)(mFaceBox[0]/mScale), (int)(mFaceBox[1]/mScale), (int)(mFaceBox[2]/mScale), (int)(mFaceBox[3]/mScale));
                canvas.drawRect(rect, paint);
                mSurfaceHolderOverlay.unlockCanvasAndPost(canvas);

                if ((ret == 0) && (mRegistrationState == REGISTRATION_START)) {
                    mRegistrationState = REGISTRATION_NOT_START;
                    startRegistrationConfirmActivity(cameraBmp);
                }
            } else {
                Canvas canvas = mSurfaceHolderOverlay.lockCanvas();
                canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
                mSurfaceHolderOverlay.unlockCanvasAndPost(canvas);
            }
        }

        private void process(CaptureResult result) {
            boolean state_switch = false;
            //Log.d(TAG, "+process:[" + mState + "]");
            if (mPrevState != mState) {
                mPrevState = mState;
                state_switch = true;
            }
            switch (mState) {
                case STATE_PREVIEW: {
                    // We have nothing to do when the camera preview is working normally.
                    break;
                }
                case STATE_WAITING_LOCK: {
                    Integer afState = result.get(CaptureResult.CONTROL_AF_STATE);
                    if (afState == null) {
                        captureStillPicture();
                    } else if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState ||
                            CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState) {
                        // CONTROL_AE_STATE can be null on some devices
                        Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                        if (aeState == null ||
                                aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED) {
                            mState = STATE_PICTURE_TAKEN;
                            captureStillPicture();
                        } else {
                            runPrecaptureSequence();
                        }
                    }
                    break;
                }
                case STATE_WAITING_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null ||
                            aeState == CaptureResult.CONTROL_AE_STATE_PRECAPTURE ||
                            aeState == CaptureRequest.CONTROL_AE_STATE_FLASH_REQUIRED) {
                        mState = STATE_WAITING_NON_PRECAPTURE;
                    }
                    break;
                }
                case STATE_WAITING_NON_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null || aeState != CaptureResult.CONTROL_AE_STATE_PRECAPTURE) {
                        mState = STATE_PICTURE_TAKEN;
                        captureStillPicture();
                    }
                    break;
                }

                case STATE_START_REGISTRATION: {
                //case STATE_TEST: {
                    process_registration();
                    break;
                }               
            }

            //Log.d(TAG, "-process:[" + mState + "]");
        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session,
                                        @NonNull CaptureRequest request,
                                        @NonNull CaptureResult partialResult) {
            process(partialResult);
        }

        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
            process(result);
        }

    };

    private static final byte SMART_LOCK = 0;
    private static final byte DOOR_ACCESS = 1;

    public Algorithm mAlgorithm;
    private int mAppType = -1;

    public int mFaceSize;
    public byte[] mFaceFeature;
    public int[] mFaceBox;
    public float mScale;

    private static final int ALGORITHM_SUPPORT_MAX_HEIGHT = 480;
    private static final int ALGORITHM_SUPPORT_MAX_WIDTH = 640;
    
    /**
     * Shows a {@link Toast} on the UI thread.
     *
     * @param text The message to show
     */
    private void showToast(final String text) {
        Activity activity = this;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(activity, text, Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the smallest one that
     * is at least as large as the respective texture view size, and that is at most as large as the
     * respective max size, and whose aspect ratio matches with the specified value. If such size
     * doesn't exist, choose the largest one that is at most as large as the respective max size,
     * and whose aspect ratio matches with the specified value.
     *
     * @param choices           The list of sizes that the camera supports for the intended output
     *                          class
     * @param textureViewWidth  The width of the texture view relative to sensor coordinate
     * @param textureViewHeight The height of the texture view relative to sensor coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @param aspectRatio       The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            Log.d(TAG, "resolution:" + option.getWidth() + "x" + option.getHeight());
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "+onCreate");
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_registration);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.capture));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        mTextureViewCamera = findViewById(R.id.texture_camera);
        mSurfaceViewOverlay = findViewById(R.id.view_overlay);
        mSurfaceViewOverlay.setZOrderOnTop(true);
        mSurfaceViewOverlay.getHolder().setFormat(PixelFormat.TRANSPARENT);
        mSurfaceHolderOverlay = mSurfaceViewOverlay.getHolder();

        Log.d(TAG, "-onCreate {" +
                "mTextureViewCamera," + mTextureViewCamera.getHeight() +":" + mTextureViewCamera.getWidth() +
                "mSurfaceViewOverlay," + mSurfaceViewOverlay.getHeight() +":" + mSurfaceViewOverlay.getWidth()
        +"}");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void setResultToActivity(int result) {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        setResultToActivity(RESULT_CANCELED);
        finish();
    }

    public void onBackFABPressed(View view) {
        setResultToActivity(RESULT_CANCELED);
        finish();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            setResultToActivity(RESULT_CANCELED);
            finish();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onPause() {
        super.onPause();

        Log.d(TAG, "+onPause");
        mState = STATE_PREVIEW;
        closeCamera();
        stopBackgroundThread();
        mAlgorithm.Exit();
        mRegistrationState = REGISTRATION_NOT_START;

        EventBus.getDefault().unregister(this);
        UserInteractionTimer.getTimerInstance().stopTimer();

        Log.d(TAG, "-onPause");
    }

    @Override
    public void onResume() {
        Log.d(TAG, "+onResume");
        super.onResume();

        EventBus.getDefault().register(this);

        startBackgroundThread();

        mAlgorithm = new Algorithm();

        // Get application typde sync with device
        BLEService.INSTANCE.sendGetAppTypeReq();

        if (mAppType != -1) {
            mAlgorithm.Init(mAppType);
            mFaceSize = mAlgorithm.GetFaceSize();
            mFaceFeature = new byte[mFaceSize];
            mFaceBox = new int[4];
        }

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (mTextureViewCamera.isAvailable()) {
           openCamera(mTextureViewCamera.getWidth(), mTextureViewCamera.getHeight());
            //openCamera(selectSize.getWidth(), selectSize.getHeight());
        } else {
            mTextureViewCamera.setSurfaceTextureListener(mSurfaceTextureListener);
        }
        mRegistrationState = REGISTRATION_NOT_START;

        UserInteractionTimer.getTimerInstance().startTimer(this);

        Log.d(TAG, "-onResume");
    }

    @Override
    public void onUserInteraction() {
        super.onUserInteraction();
        UserInteractionTimer.getTimerInstance().resetTimer();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE) {
            setResultToActivity(resultCode);
            finish();
        }
    }

    /**
     * Sets up member variables related to camera.
     *
     * @param width  The width of available size for camera preview
     * @param height The height of available size for camera preview
     */
    @SuppressWarnings("SuspiciousNameCombination")
    private void setUpCameraOutputs(int width, int height) {
        Activity activity = this;
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = manager.getCameraCharacteristics(cameraId);

                // We use a front facing camera in this sample.
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    //continue;
                    //}

                    StreamConfigurationMap map = characteristics.get(
                            CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                    if (map == null) {
                        continue;
                    }

                    // For still image captures, we use the largest available size.
                    Size largest = Collections.max(
                            Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
                            new CompareSizesByArea());
                    mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(),
                            ImageFormat.JPEG, /*maxImages*/2);
                    mImageReader.setOnImageAvailableListener(
                            mOnImageAvailableListener, mBackgroundHandler);

                    // Find out if we need to swap dimension to get the preview size relative to sensor
                    // coordinate.
                    int displayRotation = activity.getWindowManager().getDefaultDisplay().getRotation();
                    //noinspection ConstantConditions
                    mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                    boolean swappedDimensions = false;
                    switch (displayRotation) {
                        case Surface.ROTATION_0:
                        case Surface.ROTATION_180:
                            if (mSensorOrientation == 90 || mSensorOrientation == 270) {
                                swappedDimensions = true;
                            }
                            break;
                        case Surface.ROTATION_90:
                        case Surface.ROTATION_270:
                            if (mSensorOrientation == 0 || mSensorOrientation == 180) {
                                swappedDimensions = true;
                            }
                            break;
                        default:
                            Log.e(TAG, "Display rotation is invalid: " + displayRotation);
                    }

                    Point displaySize = new Point();
                    activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
                    int rotatedPreviewWidth = width;
                    int rotatedPreviewHeight = height;
                    int maxPreviewWidth = displaySize.x;
                    int maxPreviewHeight = displaySize.y;

                    if (swappedDimensions) {
                        rotatedPreviewWidth = height;
                        rotatedPreviewHeight = width;
                        maxPreviewWidth = displaySize.y;
                        maxPreviewHeight = displaySize.x;
                    }

                    if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                        maxPreviewWidth = MAX_PREVIEW_WIDTH;
                    }

                    if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                        maxPreviewHeight = MAX_PREVIEW_HEIGHT;
                    }

                    // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
                    // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
                    // garbage capture data.
                    mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                            rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                            maxPreviewHeight, largest);

                    // We fit the aspect ratio of TextureView to the size of preview we picked.
                    int orientation = getResources().getConfiguration().orientation;
                    if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                        mTextureViewCamera.setAspectRatio(
                                mPreviewSize.getWidth(), mPreviewSize.getHeight());
                        mSurfaceViewOverlay.setAspectRatio(
                                mPreviewSize.getWidth(), mPreviewSize.getHeight());
                    } else {
                        mTextureViewCamera.setAspectRatio(
                                mPreviewSize.getHeight(), mPreviewSize.getWidth());
                        mSurfaceViewOverlay.setAspectRatio(
                                mPreviewSize.getHeight(), mPreviewSize.getWidth());
                    }
                    //mTextureViewCamera.setSize(mPreviewSize.getWidth(),mPreviewSize.getHeight());

                    // Check if the flash is supported.
                    Boolean available = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
                    mFlashSupported = available == null ? false : available;

                    mCameraId = cameraId;
                    return;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on the
            // device this code runs.
            //ErrorDialog.newInstance(getString(R.string.camera_error))
            //                    .show(getChildFragmentManager(), FRAGMENT_DIALOG);
            Toast.makeText(this, "Error", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Opens the camera specified by {@link RegistrationActivity#mCameraId}.
     */
    private void openCamera(int width, int height) {
        Activity activity = this;
        if (ContextCompat.checkSelfPermission(activity, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            //requestCameraPermission();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                this.requestPermissions(new String[]{Manifest.permission.CAMERA}, 1);
            }
            return;
        }
        setUpCameraOutputs(width, height);
        configureTransform(width, height);
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            manager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mImageReader) {
                mImageReader.close();
                mImageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * Creates a new {@link CameraCaptureSession} for camera preview.
     */
    private void createCameraPreviewSession() {
        try {
            SurfaceTexture texture = mTextureViewCamera.getSurfaceTexture();
            assert texture != null;

            // We configure the size of default buffer to be the size of camera preview we want.
            texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

            // This is the output Surface we need to start preview.
            Surface surface = new Surface(texture);

            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.addTarget(surface);

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice.createCaptureSession(Arrays.asList(surface, mImageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {

                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession;
                            try {
                                // Auto focus should be continuous for camera preview.
                                mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                                // Flash is automatically enabled when necessary.
                                setAutoFlash(mPreviewRequestBuilder);

                                mState = STATE_START_REGISTRATION;
                                // Finally, we start displaying the camera preview.
                                mPreviewRequest = mPreviewRequestBuilder.build();
                                mCaptureSession.setRepeatingRequest(mPreviewRequest,
                                        mCaptureCallback, mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @Override
                        public void onConfigureFailed(
                                @NonNull CameraCaptureSession cameraCaptureSession) {
                            showToast("Failed");
                        }
                    }, null
            );

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Configures the necessary {@link android.graphics.Matrix} transformation to `mTextureViewCamera`.
     * This method should be called after the camera preview size is determined in
     * setUpCameraOutputs and also the size of `mTextureViewCamera` is fixed.
     *
     * @param viewWidth  The width of `mTextureViewCamera`
     * @param viewHeight The height of `mTextureViewCamera`
     */
    private void configureTransform(int viewWidth, int viewHeight) {

        Activity activity = this;
        if (null == mTextureViewCamera || null == mPreviewSize || null == activity) {
            return;
        }
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            float scale = Math.max(
                    (float) viewHeight / mPreviewSize.getHeight(),
                    (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (rotation - 2), centerX, centerY);
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postRotate(180, centerX, centerY);
        }
        mTextureViewCamera.setTransform(matrix);
    }

    /**
     * Initiate a still image capture.
     */
    private void takePicture() {
        lockFocus();
    }

    /**
     * Lock the focus as the first step for a still image capture.
     */
    private void lockFocus() {
        try {
            // This is how to tell the camera to lock focus.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Run the precapture sequence for capturing a still image. This method should be called when
     * we get a response in {@link #mCaptureCallback} from {@link #lockFocus()}.
     */
    private void runPrecaptureSequence() {
        try {
            // This is how to tell the camera to trigger.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Capture a still picture. This method should be called when we get a response in
     * {@link #mCaptureCallback} from both {@link #lockFocus()}.
     */
    private void captureStillPicture() {
        try {
            final Activity activity = this;
            if (null == activity || null == mCameraDevice) {
                return;
            }
            // This is the CaptureRequest.Builder that we use to take a picture.
            final CaptureRequest.Builder captureBuilder =
                    mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(mImageReader.getSurface());

            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            setAutoFlash(captureBuilder);

            // Orientation
            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, getOrientation(rotation));

            CameraCaptureSession.CaptureCallback CaptureCallback
                    = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                               @NonNull CaptureRequest request,
                                               @NonNull TotalCaptureResult result) {
                    showToast("Saved: " + mFile);
                    Log.d(TAG, mFile.toString());
                    unlockFocus();
                }
            };

            mCaptureSession.stopRepeating();
            mCaptureSession.abortCaptures();
            mCaptureSession.capture(captureBuilder.build(), CaptureCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Retrieves the JPEG orientation from the specified screen rotation.
     *
     * @param rotation The screen rotation.
     * @return The JPEG orientation (one of 0, 90, 270, and 360)
     */
    private int getOrientation(int rotation) {
        // Sensor orientation is 90 for most devices, or 270 for some devices (eg. Nexus 5X)
        // We have to take that into account and rotate JPEG properly.
        // For devices with orientation of 90, we simply return our mapping from ORIENTATIONS.
        // For devices with orientation of 270, we need to rotate the JPEG 180 degrees.
        return (ORIENTATIONS.get(rotation) + mSensorOrientation + 270) % 360;
    }

    /**
     * Unlock the focus. This method should be called when still image capture sequence is
     * finished.
     */
    private void unlockFocus() {
        try {
            // Reset the auto-focus trigger
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
            setAutoFlash(mPreviewRequestBuilder);
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback,
                    mBackgroundHandler);
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW;
            mCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback,
                    mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void setAutoFlash(CaptureRequest.Builder requestBuilder) {
        if (mFlashSupported) {
            requestBuilder.set(CaptureRequest.CONTROL_AE_MODE,
                    CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
        }
    }

    /**
     * Saves a JPEG {@link Image} into the specified {@link File}.
     */
    private static class ImageSaver implements Runnable {

        /**
         * The JPEG image
         */
        private final Image mImage;
        /**
         * The file we save the image into.
         */
        private final File mFile;

        ImageSaver(Image image, File file) {
            mImage = image;
            mFile = file;
        }

        @Override
        public void run() {
            ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
            FileOutputStream output = null;
            try {
                output = new FileOutputStream(mFile);
                output.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                mImage.close();
                if (null != output) {
                    try {
                        output.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    /**
     * Compares two {@code Size}s based on their areas.
     */
    static class CompareSizesByArea implements Comparator<Size> {
        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }
    }

    public void onClickRegistration(View view) {
        Log.d(TAG, "+onClickRegistration:" + mRegistrationState);
        if (mRegistrationState == REGISTRATION_START) {
            mRegistrationState = REGISTRATION_NOT_START;
        } else {
            mRegistrationState = REGISTRATION_START;
        }
        Log.d(TAG, "-onClickRegistration");
    }

    private String ConvertBmpImag(Bitmap bmp){
        int offset_x =  Math.max(0, mFaceBox[0] - BMP_OFFSET);
        int offset_x_prim = Math.min((mFaceBox[2] - offset_x + BMP_OFFSET), bmp.getWidth() - offset_x);
        int offset_y =  Math.max(0, mFaceBox[1] - BMP_OFFSET);
        int offset_y_prim = Math.min((mFaceBox[3] - offset_y + BMP_OFFSET), bmp.getHeight() - offset_y);

        Bitmap bitmap = Bitmap.createBitmap(bmp, offset_x, offset_y, offset_x_prim, offset_y_prim);
        Bitmap resized = Bitmap.createScaledBitmap(bitmap, 200, 200, true);
        Matrix matrix = new Matrix();
        matrix.postRotate(0);
        Bitmap rotatedBitmap = Bitmap.createBitmap(resized, 0, 0, resized.getWidth(), resized.getHeight(), matrix, true);

        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        rotatedBitmap.compress(Bitmap.CompressFormat.PNG, 100, byteArrayOutputStream);
        String strBase64Bitmap = Base64.encodeToString(byteArrayOutputStream.toByteArray(), Base64.DEFAULT);
        return strBase64Bitmap;
    }


    private void startRegistrationConfirmActivity(Bitmap bmp) {
            pendingIntent = new Intent();
            pendingIntent.setClass(this, RegistrationConfirmActivity.class);

            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, getIntent().getExtras().getInt(BaseServiceActivity.INTENT_KEY_PHY));
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_ADDRESS));
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME));
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_FEATURE, mFaceFeature);
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_IMAG, ConvertBmpImag(bmp));
            pendingIntent.putExtra(getString(R.string.intent_user_name), getIntent().getExtras().getString(getString(R.string.intent_user_name)));

            startActivityForResult(pendingIntent, REQUEST_CODE);
    }

    private void treatReturnValue(int Result){
        switch (Result){
            case SMART_LOCK:
                mAppType = SMART_LOCK;
//                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
//                        this, findViewById(R.id.registration_view), getString(R.string.success_smart_lock));
                mAlgorithm.Init(mAppType);
                mFaceSize = mAlgorithm.GetFaceSize();
                mFaceFeature = new byte[mFaceSize];
                mFaceBox = new int[4];
                break;
            case DOOR_ACCESS:
                mAppType = DOOR_ACCESS;
//                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
//                        this, findViewById(R.id.registration_view), getString(R.string.success_door_access));
                mAlgorithm.Init(mAppType);
                mFaceSize = mAlgorithm.GetFaceSize();
                mFaceFeature = new byte[mFaceSize];
                mFaceBox = new int[4];
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.registration_view), getString(R.string.error_general));
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetAppTypeRes e) {
        Log.d(TAG, "+GetAppTypeRes");

        if (e == null) return;

        treatReturnValue(e.mGetAppTypeResult);

        Log.d(TAG, "-GetAppTypeRes:" + e.mGetAppTypeResult);
    }

    // test code
    private void dumpBytes(byte[] bytes, int len) {
        Log.d(TAG, "bytes:[" + bytes.length + "]");
        int line_len = 16;
        //int bytes_len = bytes.length;
        // only 16 header + one 128 bytes face feature as the other two 128 face feature are same as the first one
        //if (face_feature_len > (128 + 16)) {
            //face_feature_len = 128 + 16;
        //}
        int lines = len / line_len;
        int remains = len % line_len;
        for (int i = 0; i < lines; i++) {
            //Log.d(TAG, bytes2HexString(faceFeature + i * line_len, line_len));
            Log.d(TAG, String.format("0x%02x ",bytes[i * line_len]) + String.format("0x%02x ",bytes[i * line_len +1]) +
                    String.format("0x%02x ",bytes[i * line_len+2]) + String.format("0x%02x ",bytes[i * line_len+3]) +
                    String.format("0x%02x ",bytes[i * line_len+4]) + String.format("0x%02x ",bytes[i * line_len+5]) +
                    String.format("0x%02x ",bytes[i * line_len+6]) + String.format("0x%02x ",bytes[i * line_len+7]) +
                    String.format("0x%02x ",bytes[i * line_len+8]) + String.format("0x%02x ",bytes[i * line_len+9]) +
                    String.format("0x%02x ",bytes[i * line_len+10]) + String.format("0x%02x ",bytes[i * line_len+11]) +
                    String.format("0x%02x ",bytes[i * line_len+12]) + String.format("0x%02x ",bytes[i * line_len+13]) +
                    String.format("0x%02x ",bytes[i * line_len+14]) + String.format("0x%02x ",bytes[i * line_len+15]));
        }
    }

    private int testGenFaceFeature(byte[] faceFeature) {
        FileInputStream fis = null;
        try
        {
            fis = new FileInputStream("/sdcard/facemanager/one_face_480_640.png");
        }
        catch (FileNotFoundException ex)
        {
            // insert code to run when exception occurs
            Log.d(TAG, "FILE LOAD ERROR");
        }

        Bitmap orig_bitmap = BitmapFactory.decodeStream(fis);
        Bitmap scaled_bitmap;

        int width = orig_bitmap.getWidth();
        int height = orig_bitmap.getHeight();
        Log.d(TAG, "original image wxh:" + width + "x" + height);

        if (width > 480 || height > 640){
            float scaleWidth = ((float) 480)/width;
            float scaleHeight = ((float) 640)/height;
            float scale = Math.min(scaleHeight, scaleWidth);
            Matrix matrix = new Matrix();
            matrix.setScale(scale, scale);
            scaled_bitmap = Bitmap.createBitmap(orig_bitmap, 0,0,width, height, matrix, false);
            width  = scaled_bitmap.getWidth();
            height = scaled_bitmap.getHeight();
            Log.e(TAG, "scaled image wxh:" + width + "x" + height);
            orig_bitmap = scaled_bitmap;
        }

        final Bitmap grayBitmap = rgbToGrayscale(orig_bitmap);

        byte[] FrameData = getPixelsRGB(grayBitmap);

        //byte[] FrameData = new byte[480*640*3];
        int Width = 480;
        int Height = 640;
        int[] faceBox = new int[4];
        int ret = mAlgorithm.Registration(FrameData, Width, Height, faceBox, faceFeature);

        Log.d(TAG, "[Algorithm]:run:ret:[" + ret + "]:mFaceBox:["+ faceBox[0] + "," + faceBox[1] + "," + faceBox[2] + "," + faceBox[3] + "]");

        dumpBytes(faceFeature, 128 + 16);

        return ret;
    }

}
