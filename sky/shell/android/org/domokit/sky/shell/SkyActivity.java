// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.domokit.sky.shell;

import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;

import org.chromium.base.PathUtils;
import org.chromium.mojom.sky.EventType;
import org.chromium.mojom.sky.InputEvent;

import org.domokit.activity.ActivityImpl;

import java.io.File;

/**
 * Base class for activities that use Sky.
 */
public class SkyActivity extends Activity {
    private TracingController mTracingController;
    private PlatformViewAndroid mView;

    private String[] getArgsFromIntent(Intent intent) {
        // Before adding more entries to this list, consider that arbitrary
        // Android applications can generate intents with extra data and that
        // there are many security-sensitive args in the binary.
        if (intent.getBooleanExtra("enable-checked-mode", false)) {
            String[] args = { "--enable-checked-mode"};
            return args;
        }
        return null;
    }

    /**
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        PlatformViewAndroid.EdgeDims edgeDims = new PlatformViewAndroid.EdgeDims();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().setStatusBarColor(0x40000000);
        }
        // TODO(abarth): We should get this value from the Android framework somehow.
        edgeDims.top = 25.0;
        // TODO(abarth): Unclear if we want to use fullscreen if we don't have
        // a transparent system bar.
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);

        String[] args = getArgsFromIntent(getIntent());
        SkyMain.ensureInitialized(getApplicationContext(), args);
        mView = new PlatformViewAndroid(this, edgeDims);
        ActivityImpl.setCurrentActivity(this);
        setContentView(mView);
        mTracingController = new TracingController(this);

        onSkyReady();
    }

    /**
     * @see android.app.Activity#onDestroy()
     */
    @Override
    protected void onDestroy() {
        // Do we need to shut down Sky too?
        mTracingController.stop();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (mView != null) {
            InputEvent event = new InputEvent();
            event.type = EventType.BACK;
            mView.getEngine().onInputEvent(event);
            return;
        }
        super.onBackPressed();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mView != null) {
            mView.getEngine().onActivityPaused();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        ActivityImpl.onResumeActivity(this);
    }

    @Override
    protected void onPostResume() {
        super.onPostResume();
        if (mView != null) {
            mView.getEngine().onActivityResumed();
        }
    }

    /**
      * Override this function to customize startup behavior.
      */
    protected void onSkyReady() {
        if (loadIntent(getIntent())) {
            return;
        }
        File dataDir = new File(PathUtils.getDataDirectory(this));
        File snapshot = new File(dataDir, SkyApplication.SNAPSHOT);
        if (snapshot.exists()) {
            mView.getEngine().runFromSnapshot(snapshot.getPath());
            return;
        }
        File appBundle = new File(dataDir, SkyApplication.APP_BUNDLE);
        if (appBundle.exists()) {
            mView.getEngine().runFromBundle(appBundle.getPath());
            return;
        }
    }

    protected void onNewIntent(Intent intent) {
        loadIntent(intent);
    }

    public boolean loadIntent(Intent intent) {
        String action = intent.getAction();

        if (Intent.ACTION_VIEW.equals(action)) {
            loadUrl(intent.getDataString());
            return true;
        }

        if (Intent.ACTION_RUN.equals(action)) {
            mView.getEngine().runFromBundle(intent.getDataString());
            return true;
        }

        return false;
    }

    public void loadUrl(String url) {
        mView.getEngine().runFromNetwork(url);
    }

    public boolean loadBundleByName(String name) {
        File dataDir = new File(PathUtils.getDataDirectory(this));
        File bundle = new File(dataDir, name);
        if (!bundle.exists()) {
            return false;
        }
        mView.getEngine().runFromBundle(bundle.getPath());
        return true;
    }
}
