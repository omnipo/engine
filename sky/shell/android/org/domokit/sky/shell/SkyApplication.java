// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.domokit.sky.shell;

import android.content.Context;
import android.util.Log;

import org.chromium.base.BaseChromiumApplication;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.base.library_loader.ProcessInitException;
import org.chromium.base.PathUtils;
import org.chromium.mojo.keyboard.KeyboardServiceImpl;
import org.chromium.mojo.sensors.SensorServiceImpl;
import org.chromium.mojo.system.Core;
import org.chromium.mojo.system.MessagePipeHandle;
import org.chromium.mojom.activity.Activity;
import org.chromium.mojom.activity.PathService;
import org.chromium.mojom.keyboard.KeyboardService;
import org.chromium.mojom.media.MediaService;
import org.chromium.mojom.mojo.NetworkService;
import org.chromium.mojom.sensors.SensorService;
import org.chromium.mojom.vsync.VSyncProvider;
import org.domokit.activity.ActivityImpl;
import org.domokit.activity.PathServiceImpl;
import org.domokit.media.MediaServiceImpl;
import org.domokit.oknet.NetworkServiceImpl;
import org.domokit.vsync.VSyncProviderImpl;

/**
 * Sky implementation of {@link android.app.Application}, managing application-level global
 * state and initializations.
 */
public class SkyApplication extends BaseChromiumApplication {
    static final String SNAPSHOT = "snapshot_blob.bin";
    static final String APP_BUNDLE = "app.flx";
    static final String MANIFEST = "flutter.yaml";

    private static final String TAG = "SkyApplication";
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "sky_shell";
    private static final String[] SKY_RESOURCES = {"icudtl.dat", SNAPSHOT, APP_BUNDLE, MANIFEST};

    private ResourceExtractor mResourceExtractor;

    public ResourceExtractor getResourceExtractor() {
        return mResourceExtractor;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        initJavaUtils();
        initResources();
        initNative();
        UpdateService.init(getApplicationContext());
        onServiceRegistryAvailable(ServiceRegistry.SHARED);
    }

    /**
      * Override this function to add more resources for extraction.
      */
    protected void onBeforeResourceExtraction(ResourceExtractor extractor) {
        extractor.addResources(SKY_RESOURCES);
    }

    /**
      * Override this function to register more services.
      */
    protected void onServiceRegistryAvailable(ServiceRegistry registry) {
        registry.register(Activity.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                Activity.MANAGER.bind(new ActivityImpl(), pipe);
            }
        });

        registry.register(org.chromium.mojom.updater.UpdateService.MANAGER.getName(),
                          new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                org.chromium.mojom.updater.UpdateService.MANAGER.bind(
                    new UpdateService.MojoService(), pipe);
            }
        });

        registry.register(PathService.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                PathService.MANAGER.bind(new PathServiceImpl(getApplicationContext()), pipe);
            }
        });

        registry.register(KeyboardService.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                KeyboardService.MANAGER.bind(new KeyboardServiceImpl(context), pipe);
            }
        });

        registry.register(MediaService.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                MediaService.MANAGER.bind(new MediaServiceImpl(context, core), pipe);
            }
        });

        registry.register(NetworkService.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                NetworkService.MANAGER.bind(new NetworkServiceImpl(context, core), pipe);
            }
        });

        registry.register(SensorService.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                SensorService.MANAGER.bind(new SensorServiceImpl(context), pipe);
            }
        });

        registry.register(VSyncProvider.MANAGER.getName(), new ServiceFactory() {
            @Override
            public void connectToService(Context context, Core core, MessagePipeHandle pipe) {
                VSyncProvider.MANAGER.bind(new VSyncProviderImpl(pipe), pipe);
            }
        });
    }

    private void initJavaUtils() {
        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX,
                                                getApplicationContext());
    }

    private void initResources() {
        Context context = getApplicationContext();
        new ResourceCleaner(context).start();
        mResourceExtractor = new ResourceExtractor(context);
        onBeforeResourceExtraction(mResourceExtractor);
        mResourceExtractor.start();
    }

    private void initNative() {
        try {
            LibraryLoader.get(LibraryProcessType.PROCESS_BROWSER).ensureInitialized(this);
        } catch (ProcessInitException e) {
            Log.e(TAG, "Unable to load Sky Engine binary.", e);
            throw new RuntimeException(e);
        }
    }
}
