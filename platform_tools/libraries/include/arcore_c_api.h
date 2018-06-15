/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ARCORE_C_API_H_
#define ARCORE_C_API_H_

#include <stddef.h>
#include <stdint.h>

/// @defgroup concepts Concepts
/// High-Level concepts of ARCore
///
/// @section ownership Object ownership
///
/// ARCore has two categories of objects: "value types" and "reference types".
///
/// - Value types are owned by application. They are created and destroyed using
///   the @c create / @c destroy methods, and are populated by ARCore using
///   methods with @c get in the method name.
///
/// - Reference types are owned by ARCore. A reference is acquired by one of the
///   @c acquire methods.  For each call to the @c acquire method, the
///   application must call the matching @c release method. Note that even if
///   last reference is released, ARCore may continue to hold a reference to the
///   object at ARCore's discretion.
///
/// Reference types are further split into:
///
/// - Long-lived objects. These objects persist across frames, possibly for the
///   life span of the application or session. Acquire may fail if ARCore is in
///   an incorrect state, e.g. not tracking. Acquire from list always succeeds,
///   as the object already exists.
///
/// - Transient large data. These objects are usually acquired per-frame and are
///   a limited resource. The @c acquire call may fail if the resource is
///   exhausted (too many are currently held), deadline exceeded (the target of
///   the acquire was already released), or the resource is not yet available.
///
/// Note: Lists are value types (owned by application), but can hold references
/// to long-lived objects. This means that the references held by a list are not
/// released until either the list is destroyed, or is re-populated by another
/// api call.
///
/// For example, ::ArAnchorList, which is a value type, will hold references to
/// Anchors, which are long-lived objects.
///
/// @section spaces Poses and Coordinate Spaces
///
/// An @c ArPose describes an rigid transformation from one coordinate space to
/// another. As provided from all ARCore APIs, Poses always describe the
/// transformation from object's local coordinate space to the <b>world
/// coordinate space</b> (see below). That is, Poses from ARCore APIs can be
/// thought of as equivalent to OpenGL model matrices.
///
/// The transformation is defined using a quaternion rotation about the origin
/// followed by a translation.
///
/// The coordinate system is right-handed, like OpenGL conventions.
///
/// Translation units are meters.
///
/// @section worldcoordinates World Coordinate Space
///
/// As ARCore's understanding of the environment changes, it adjusts its model
/// of the world to keep things consistent. When this happens, the numerical
/// location (coordinates) of the camera and anchors can change significantly to
/// maintain appropriate relative positions of the physical locations they
/// represent.
///
/// These changes mean that every frame should be considered to be in a
/// completely unique world coordinate space. The numerical coordinates of
/// anchors and the camera should never be used outside the rendering frame
/// during which they were retrieved. If a position needs to be considered
/// beyond the scope of a single rendering frame, either an anchor should be
/// created or a position relative to a nearby existing anchor should be used.

/// @defgroup common Common Definitions
/// Shared types and constants

/// @defgroup anchor Anchor
/// Describes a fixed location and orientation in the real world.

/// @defgroup arcoreapk ArCoreApk
/// Management of the ARCore service APK

/// @defgroup augmented_image AugmentedImage
/// An image being detected and tracked by ARCore.

/// @defgroup augmented_image_database AugmentedImageDatabase
/// Database containing a list of images to be detected and tracked by ARCore.

/// @defgroup camera Camera
/// Provides information about the camera that is used to capture images.

/// @defgroup cloud Cloud Anchors
/// The cloud state and configuration of an Anchor and the AR Session.

/// @defgroup config Configuration
/// Session configuration.

/// @defgroup frame Frame
/// Per-frame state.

/// @defgroup hit HitResult
/// Defines an intersection between a ray and estimated real-world geometry.

/// @defgroup image ImageMetadata
/// Provides access to metadata from the camera image capture result.

/// @defgroup light LightEstimate
/// Holds information about the estimated lighting of the real scene.

/// @defgroup plane Plane
/// Describes the current best knowledge of a real-world planar surface.

/// @defgroup point Point
/// Represents a point in space that ARCore is tracking.

/// @defgroup pointcloud PointCloud
/// Contains a set of observed 3D points and confidence values.

/// @defgroup pose Pose
/// Represents an immutable rigid transformation from one coordinate
/// space to another.

/// @defgroup session Session
/// Session management.

/// @defgroup trackable Trackable
/// Something that can be tracked and that Anchors can be attached to.

/// @defgroup cpp_helpers C++ helper functions

/// @addtogroup config
/// @{

/// An opaque session configuration object (@ref ownership "value type").
///
/// Create with ArConfig_create()<br>
/// Release with ArConfig_destroy()
typedef struct ArConfig_ ArConfig;

/// @}

/// @addtogroup session
/// @{

/// The ArCore session (@ref ownership "value type").
///
/// Create with ArSession_create()<br>
/// Release with ArSession_destroy()
typedef struct ArSession_ ArSession;

/// @}

/// @addtogroup pose
/// @{

/// A structured rigid transformation (@ref ownership "value type").
///
/// Allocate with ArPose_create()<br>
/// Release with ArPose_destroy()
typedef struct ArPose_ ArPose;

/// @}

// Camera.

/// @addtogroup camera
/// @{

/// The virtual and physical camera
/// (@ref ownership "reference type, long-lived").
///
/// Acquire with ArFrame_acquireCamera()<br>
/// Release with ArCamera_release()
typedef struct ArCamera_ ArCamera;

/// @}

// Frame and frame objects.

/// @addtogroup frame
/// @{

/// The world state resulting from an update (@ref ownership "value type").
///
/// Allocate with ArFrame_create()<br>
/// Populate with ArSession_update()<br>
/// Release with ArFrame_destroy()
typedef struct ArFrame_ ArFrame;

/// @}

// LightEstimate.

/// @addtogroup light
/// @{

/// An estimate of the real-world lighting (@ref ownership "value type").
///
/// Allocate with ArLightEstimate_create()<br>
/// Populate with ArFrame_getLightEstimate()<br>
/// Release with ArLightEstimate_destroy()
typedef struct ArLightEstimate_ ArLightEstimate;

/// @}

// PointCloud.

/// @addtogroup pointcloud
/// @{

/// A cloud of tracked 3D visual feature points
/// (@ref ownership "reference type, large data").
///
/// Acquire with ArFrame_acquirePointCloud()<br>
/// Release with ArPointCloud_release()
typedef struct ArPointCloud_ ArPointCloud;

/// @}

// ImageMetadata.

/// @addtogroup image
/// @{

/// Camera capture metadata (@ref ownership "reference type, large data").
///
/// Acquire with ArFrame_acquireImageMetadata()<br>
/// Release with ArImageMetadata_release()
typedef struct ArImageMetadata_ ArImageMetadata;

/// Accessing CPU image from the tracking camera
/// (@ref ownership "reference type, large data").
///
/// Acquire with ArFrame_acquireCameraImage()<br>
/// Convert to NDK AImage with ArImage_getNdkImage()<br>
/// Release with ArImage_release().
typedef struct ArImage_ ArImage;

/// Forward declaring the AImage struct from Android NDK, which is used
/// in ArImage_getNdkImage().
typedef struct AImage AImage;
/// @}

// Trackables.

/// @addtogroup trackable
/// @{

/// Trackable base type (@ref ownership "reference type, long-lived").
typedef struct ArTrackable_ ArTrackable;

/// A list of ArTrackables (@ref ownership "value type").
///
/// Allocate with ArTrackableList_create()<br>
/// Release with ArTrackableList_destroy()
typedef struct ArTrackableList_ ArTrackableList;

/// @}

// Plane

/// @addtogroup plane
/// @{

/// A detected planar surface (@ref ownership "reference type, long-lived").
///
/// Trackable type: #AR_TRACKABLE_PLANE <br>
/// Release with: ArTrackable_release()
typedef struct ArPlane_ ArPlane;

/// @}

// Point

/// @addtogroup point
/// @{

/// An arbitrary point in space (@ref ownership "reference type, long-lived").
///
/// Trackable type: #AR_TRACKABLE_POINT <br>
/// Release with: ArTrackable_release()
typedef struct ArPoint_ ArPoint;

/// @}

// Augmented Image

/// @addtogroup augmented_image
/// @{

/// An image that has been detected and tracked (@ref ownership "reference type,
/// long-lived").
///
/// Trackable type: #AR_TRACKABLE_AUGMENTED_IMAGE <br>
/// Release with: ArTrackable_release()
typedef struct ArAugmentedImage_ ArAugmentedImage;

/// @}

// Augmented Image Database
/// @addtogroup augmented_image_database
/// @{

/// A database of images to be detected and tracked by ARCore (@ref ownership
/// "value type").
///
/// An image database supports up to 1000 images. A database can be generated by
/// the `arcoreimg` command-line database generation tool provided in the SDK,
/// or dynamically created at runtime by adding individual images.
///
/// Only one image database can be active in a session. Any images in the
/// currently active image database that have a TRACKING/PAUSED state will
/// immediately be set to the STOPPED state if a different or null image
/// database is made active in the current session Config.
///
/// Create with ArAugmentedImageDatabase_create() or
/// ArAugmentedImageDatabase_deserialize()<br>
/// Release with: ArAugmentedImageDatabase_destroy()
typedef struct ArAugmentedImageDatabase_ ArAugmentedImageDatabase;

/// @}

// Anchors.

/// @addtogroup anchor
/// @{

/// A position in space attached to a trackable
/// (@ref ownership "reference type, long-lived").
///
/// Create with ArSession_acquireNewAnchor() or
///     ArHitResult_acquireNewAnchor()<br>
/// Release with ArAnchor_release()
typedef struct ArAnchor_ ArAnchor;

/// A list of anchors (@ref ownership "value type").
///
/// Allocate with ArAnchorList_create()<br>
/// Release with ArAnchorList_destroy()
typedef struct ArAnchorList_ ArAnchorList;

/// @}

// Hit result functionality.

/// @addtogroup hit
/// @{

/// A single trackable hit (@ref ownership "value type").
///
/// Allocate with ArHitResult_create()<br>
/// Populate with ArHitResultList_getItem()<br>
/// Release with ArHitResult_destroy()
typedef struct ArHitResult_ ArHitResult;

/// A list of hit test results (@ref ownership "value type").
///
/// Allocate with ArHitResultList_create()<br>
/// Release with ArHitResultList_destroy()<br>
typedef struct ArHitResultList_ ArHitResultList;

/// @}

/// @cond EXCLUDE_FROM_DOXYGEN

// Forward declaring the ACameraMetadata struct from Android NDK, which is used
// in ArImageMetadata_getNdkCameraMetadata
typedef struct ACameraMetadata ACameraMetadata;

/// @endcond

/// @addtogroup cpp_helpers
/// @{
/// These methods expose allowable type conversions as C++ helper functions.
/// This avoids having to explicitly @c reinterpret_cast in most cases.
///
/// Note: These methods only change the type of a pointer - they do not change
/// the reference count of the referenced objects.
///
/// Note: There is no runtime checking that casts are correct. Call @ref
/// ArTrackable_getType() beforehand to figure out the correct cast.

#ifdef __cplusplus
/// Upcasts to ArTrackable
inline ArTrackable *ArAsTrackable(ArPlane *plane) {
  return reinterpret_cast<ArTrackable *>(plane);
}

/// Upcasts to ArTrackable
inline ArTrackable *ArAsTrackable(ArPoint *point) {
  return reinterpret_cast<ArTrackable *>(point);
}

/// Upcasts to ArTrackable
inline ArTrackable *ArAsTrackable(ArAugmentedImage *augmented_image) {
  return reinterpret_cast<ArTrackable *>(augmented_image);
}

/// Downcasts to ArPlane.
inline ArPlane *ArAsPlane(ArTrackable *trackable) {
  return reinterpret_cast<ArPlane *>(trackable);
}

/// Downcasts to ArPoint.
inline ArPoint *ArAsPoint(ArTrackable *trackable) {
  return reinterpret_cast<ArPoint *>(trackable);
}

/// Downcasts to ArAugmentedImage.
inline ArAugmentedImage *ArAsAugmentedImage(ArTrackable *trackable) {
  return reinterpret_cast<ArAugmentedImage *>(trackable);
}
#endif
/// @}

// If compiling for C++11, use the 'enum underlying type' feature to enforce
// size for ABI compatibility. In pre-C++11, use int32_t for fixed size.
#if __cplusplus >= 201100
#define AR_DEFINE_ENUM(_type) enum _type : int32_t
#else
#define AR_DEFINE_ENUM(_type) \
  typedef int32_t _type;      \
  enum
#endif

#if defined(__GNUC__) && !defined(AR_DEPRECATED_SUPPRESS)
#define AR_DEPRECATED(_deprecation_string) \
  __attribute__((deprecated(_deprecation_string)));
#else
#define AR_DEPRECATED(_deprecation_string)
#endif

/// @ingroup trackable
/// Object types for heterogeneous query/update lists.
AR_DEFINE_ENUM(ArTrackableType){
    /// The base Trackable type. Can be passed to ArSession_getAllTrackables()
    /// and ArFrame_getUpdatedTrackables() as the @c filter_type to get
    /// all/updated Trackables of all types.
    AR_TRACKABLE_BASE_TRACKABLE = 0x41520100,

    /// The ::ArPlane subtype of Trackable.
    AR_TRACKABLE_PLANE = 0x41520101,

    /// The ::ArPoint subtype of Trackable.
    AR_TRACKABLE_POINT = 0x41520102,

    /// The ::ArAugmentedImage subtype of Trackable.
    AR_TRACKABLE_AUGMENTED_IMAGE = 0x41520104,

    /// An invalid Trackable type.
    AR_TRACKABLE_NOT_VALID = 0};

/// @ingroup common
/// Return code indicating success or failure of a method.
AR_DEFINE_ENUM(ArStatus){
    /// The operation was successful.
    AR_SUCCESS = 0,

    /// One of the arguments was invalid, either null or not appropriate for the
    /// operation requested.
    AR_ERROR_INVALID_ARGUMENT = -1,

    /// An internal error occurred that the application should not attempt to
    /// recover from.
    AR_ERROR_FATAL = -2,

    /// An operation was attempted that requires the session be running, but the
    /// session was paused.
    AR_ERROR_SESSION_PAUSED = -3,

    /// An operation was attempted that requires the session be paused, but the
    /// session was running.
    AR_ERROR_SESSION_NOT_PAUSED = -4,

    /// An operation was attempted that the session be in the TRACKING state,
    /// but the session was not.
    AR_ERROR_NOT_TRACKING = -5,

    /// A texture name was not set by calling ArSession_setCameraTextureName()
    /// before the first call to ArSession_update()
    AR_ERROR_TEXTURE_NOT_SET = -6,

    /// An operation required GL context but one was not available.
    AR_ERROR_MISSING_GL_CONTEXT = -7,

    /// The configuration supplied to ArSession_configure() was unsupported.
    /// To avoid this error, ensure that Session_checkSupported() returns true.
    AR_ERROR_UNSUPPORTED_CONFIGURATION = -8,

    /// The android camera permission has not been granted prior to calling
    /// ArSession_resume()
    AR_ERROR_CAMERA_PERMISSION_NOT_GRANTED = -9,

    /// Acquire failed because the object being acquired is already released.
    /// For example, this happens if the application holds an ::ArFrame beyond
    /// the next call to ArSession_update(), and then tries to acquire its point
    /// cloud.
    AR_ERROR_DEADLINE_EXCEEDED = -10,

    /// There are no available resources to complete the operation.  In cases of
    /// @c acquire methods returning this error, This can be avoided by
    /// releasing previously acquired objects before acquiring new ones.
    AR_ERROR_RESOURCE_EXHAUSTED = -11,

    /// Acquire failed because the data isn't available yet for the current
    /// frame. For example, acquire the image metadata may fail with this error
    /// because the camera hasn't fully started.
    AR_ERROR_NOT_YET_AVAILABLE = -12,

    /// The android camera has been reallocated to a higher priority app or is
    /// otherwise unavailable.
    AR_ERROR_CAMERA_NOT_AVAILABLE = -13,

    /// The host/resolve function call failed because the Session is not
    /// configured for cloud anchors.
    AR_ERROR_CLOUD_ANCHORS_NOT_CONFIGURED = -14,

    /// ArSession_configure() failed because the specified configuration
    /// required the Android INTERNET permission, which the application did not
    /// have.
    AR_ERROR_INTERNET_PERMISSION_NOT_GRANTED = -15,

    /// HostCloudAnchor() failed because the anchor is not a type of anchor that
    /// is currently supported for hosting.
    AR_ERROR_ANCHOR_NOT_SUPPORTED_FOR_HOSTING = -16,

    /// An image with insufficient quality (e.g. too few features) was attempted
    /// to be added to the image database.
    AR_ERROR_IMAGE_INSUFFICIENT_QUALITY = -17,

    /// The data passed in for this operation was not in a valid format.
    AR_ERROR_DATA_INVALID_FORMAT = -18,

    /// The data passed in for this operation is not supported by this version
    /// of the SDK.
    AR_ERROR_DATA_UNSUPPORTED_VERSION = -19,

    /// The ARCore APK is not installed on this device.
    AR_UNAVAILABLE_ARCORE_NOT_INSTALLED = -100,

    /// The device is not currently compatible with ARCore.
    AR_UNAVAILABLE_DEVICE_NOT_COMPATIBLE = -101,

    /// The ARCore APK currently installed on device is too old and needs to be
    /// updated.
    AR_UNAVAILABLE_APK_TOO_OLD = -103,

    /// The ARCore APK currently installed no longer supports the ARCore SDK
    /// that the application was built with.
    AR_UNAVAILABLE_SDK_TOO_OLD = -104,

    /// The user declined installation of the ARCore APK during this run of the
    /// application and the current request was not marked as user-initiated.
    AR_UNAVAILABLE_USER_DECLINED_INSTALLATION = -105};

/// @ingroup common
/// Describes the tracking state of a @c Trackable, an ::ArAnchor or the
/// ::ArCamera.
AR_DEFINE_ENUM(ArTrackingState){
    /// The object is currently tracked and its pose is current.
    AR_TRACKING_STATE_TRACKING = 0,

    /// ARCore has paused tracking this object, but may resume tracking it in
    /// the future. This can happen if device tracking is lost, if the user
    /// enters a new space, or if the Session is currently paused. When in this
    /// state, the positional properties of the object may be wildly inaccurate
    /// and should not be used.
    AR_TRACKING_STATE_PAUSED = 1,

    /// ARCore has stopped tracking this Trackable and will never resume
    /// tracking it.
    AR_TRACKING_STATE_STOPPED = 2};

/// @ingroup cloud
/// Describes the current cloud state of an @c Anchor.
AR_DEFINE_ENUM(ArCloudAnchorState){
    /// The anchor is purely local. It has never been hosted using
    /// hostCloudAnchor, and has not been acquired using acquireCloudAnchor.
    AR_CLOUD_ANCHOR_STATE_NONE = 0,

    /// A hosting/resolving task for the anchor is in progress. Once the task
    /// completes in the background, the anchor will get a new cloud state after
    /// the next update() call.
    AR_CLOUD_ANCHOR_STATE_TASK_IN_PROGRESS = 1,

    /// A hosting/resolving task for this anchor completed successfully.
    AR_CLOUD_ANCHOR_STATE_SUCCESS = 2,

    /// A hosting/resolving task for this anchor finished with an internal
    /// error. The app should not attempt to recover from this error.
    AR_CLOUD_ANCHOR_STATE_ERROR_INTERNAL = -1,

    /// The app cannot communicate with the ARCore Cloud because of an invalid
    /// or unauthorized API key in the manifest, or because there was no API key
    /// present in the manifest.
    AR_CLOUD_ANCHOR_STATE_ERROR_NOT_AUTHORIZED = -2,

    /// The ARCore Cloud was unreachable. This can happen because of a number of
    /// reasons. The request sent to the server could have timed out with no
    /// response, there could be a bad network connection, DNS unavailability,
    /// firewall issues, or anything that could affect the device's ability to
    /// connect to the ARCore Cloud.
    AR_CLOUD_ANCHOR_STATE_ERROR_SERVICE_UNAVAILABLE = -3,

    /// The application has exhausted the request quota allotted to the given
    /// API key. The developer should request additional quota for the ARCore
    /// Cloud for their API key from the Google Developers Console.
    AR_CLOUD_ANCHOR_STATE_ERROR_RESOURCE_EXHAUSTED = -4,

    /// Hosting failed, because the server could not successfully process the
    /// dataset for the given anchor. The developer should try again after the
    /// device has gathered more data from the environment.
    AR_CLOUD_ANCHOR_STATE_ERROR_HOSTING_DATASET_PROCESSING_FAILED = -5,

    /// Resolving failed, because the ARCore Cloud could not find the provided
    /// cloud anchor ID.
    AR_CLOUD_ANCHOR_STATE_ERROR_CLOUD_ID_NOT_FOUND = -6,

    /// The server could not match the visual features provided by ARCore
    /// against the localization dataset of the requested cloud anchor ID. This
    /// means that the anchor pose being requested was likely not created in the
    /// user's surroundings.
    AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_LOCALIZATION_NO_MATCH = -7,

    /// The anchor could not be resolved because the SDK used to host the anchor
    /// was newer than and incompatible with the version being used to acquire
    /// it.
    AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_SDK_VERSION_TOO_OLD = -8,

    /// The anchor could not be acquired because the SDK used to host the anchor
    /// was older than and incompatible with the version being used to acquire
    /// it.
    AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_SDK_VERSION_TOO_NEW = -9};

/// @ingroup arcoreapk
/// Describes the current state of ARCore availability on the device.
AR_DEFINE_ENUM(ArAvailability){
    /// An internal error occurred while determining ARCore availability.
    AR_AVAILABILITY_UNKNOWN_ERROR = 0,
    /// ARCore is not installed, and a query has been issued to check if ARCore
    /// is is supported.
    AR_AVAILABILITY_UNKNOWN_CHECKING = 1,
    /// ARCore is not installed, and the query to check if ARCore is supported
    /// timed out. This may be due to the device being offline.
    AR_AVAILABILITY_UNKNOWN_TIMED_OUT = 2,
    /// ARCore is not supported on this device.
    AR_AVAILABILITY_UNSUPPORTED_DEVICE_NOT_CAPABLE = 100,
    /// The device and Android version are supported, but the ARCore APK is not
    /// installed.
    AR_AVAILABILITY_SUPPORTED_NOT_INSTALLED = 201,
    /// The device and Android version are supported, and a version of the
    /// ARCore APK is installed, but that ARCore APK version is too old.
    AR_AVAILABILITY_SUPPORTED_APK_TOO_OLD = 202,
    /// ARCore is supported, installed, and available to use.
    AR_AVAILABILITY_SUPPORTED_INSTALLED = 203};

/// @ingroup arcoreapk
/// Indicates the outcome of a call to ArCoreApk_requestInstall().
AR_DEFINE_ENUM(ArInstallStatus){
    /// The requested resource is already installed.
    AR_INSTALL_STATUS_INSTALLED = 0,
    /// Installation of the resource was requested. The current activity will be
    /// paused.
    AR_INSTALL_STATUS_INSTALL_REQUESTED = 1};

/// @ingroup arcoreapk
/// Controls the behavior of the installation UI.
AR_DEFINE_ENUM(ArInstallBehavior){
    /// Hide the Cancel button during initial prompt and prevent user from
    /// exiting via tap-outside.
    ///
    /// Note: The BACK button or tapping outside of any marketplace-provided
    /// install dialog will still decline the installation.
    AR_INSTALL_BEHAVIOR_REQUIRED = 0,
    /// Include Cancel button in initial prompt and allow easily backing out
    /// after installation has been initiated.
    AR_INSTALL_BEHAVIOR_OPTIONAL = 1};

/// @ingroup arcoreapk
/// Controls the message displayed by the installation UI.
AR_DEFINE_ENUM(ArInstallUserMessageType){
    /// Display a localized message like "This application requires ARCore...".
    AR_INSTALL_USER_MESSAGE_TYPE_APPLICATION = 0,
    /// Display a localized message like "This feature requires ARCore...".
    AR_INSTALL_USER_MESSAGE_TYPE_FEATURE = 1,
    /// Application has explained why ARCore is required prior to calling
    /// ArCoreApk_requestInstall(), skip user education dialog.
    AR_INSTALL_USER_MESSAGE_TYPE_USER_ALREADY_INFORMED = 2};

/// @ingroup config
/// Select the behavior of the lighting estimation subsystem.
AR_DEFINE_ENUM(ArLightEstimationMode){
    /// Lighting estimation is disabled.
    AR_LIGHT_ESTIMATION_MODE_DISABLED = 0,
    /// Lighting estimation is enabled, generating a single-value intensity
    /// estimate.
    AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY = 1};

/// @ingroup config
/// Select the behavior of the plane detection subsystem.
AR_DEFINE_ENUM(ArPlaneFindingMode){
    /// Plane detection is disabled.
    AR_PLANE_FINDING_MODE_DISABLED = 0,
    /// Detection of only horizontal planes is enabled.
    AR_PLANE_FINDING_MODE_HORIZONTAL = 1,
    /// Detection of only vertical planes is enabled.
    AR_PLANE_FINDING_MODE_VERTICAL = 2,
    /// Detection of horizontal and vertical planes is enabled.
    AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL = 3};

/// @ingroup config
/// Selects the behavior of ArSession_update().
AR_DEFINE_ENUM(ArUpdateMode){
    /// @c update() will wait until a new camera image is available, or until
    /// the built-in timeout (currently 66ms) is reached. On most
    /// devices the camera is configured to capture 30 frames per second.
    /// If the camera image does not arrive by the built-in timeout, then
    /// @c update() will return the most recent ::ArFrame object.
    AR_UPDATE_MODE_BLOCKING = 0,
    /// @c update() will return immediately without blocking. If no new camera
    /// image is available, then @c update() will return the most recent
    /// ::ArFrame object.
    AR_UPDATE_MODE_LATEST_CAMERA_IMAGE = 1};

/// @ingroup plane
/// Simple summary of the normal vector of a plane, for filtering purposes.
AR_DEFINE_ENUM(ArPlaneType){
    /// A horizontal plane facing upward (for example a floor or tabletop).
    AR_PLANE_HORIZONTAL_UPWARD_FACING = 0,
    /// A horizontal plane facing downward (for example a ceiling).
    AR_PLANE_HORIZONTAL_DOWNWARD_FACING = 1,
    /// A vertical plane (for example a wall).
    AR_PLANE_VERTICAL = 2};

/// @ingroup light
/// Tracks the validity of a light estimate.
AR_DEFINE_ENUM(ArLightEstimateState){
    /// The light estimate is not valid this frame and should not be used for
    /// rendering.
    AR_LIGHT_ESTIMATE_STATE_NOT_VALID = 0,
    /// The light estimate is valid this frame.
    AR_LIGHT_ESTIMATE_STATE_VALID = 1};

/// @ingroup point
/// Indicates the orientation mode of the ::ArPoint.
AR_DEFINE_ENUM(ArPointOrientationMode){
    /// The orientation of the ::ArPoint is initialized to identity but may
    /// adjust slightly over time.
    AR_POINT_ORIENTATION_INITIALIZED_TO_IDENTITY = 0,
    /// The orientation of the ::ArPoint will follow the behavior described in
    /// ArHitResult_getHitPose().
    AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL = 1};

/// @ingroup cloud
/// Indicates the cloud configuration of the ::ArSession.
AR_DEFINE_ENUM(ArCloudAnchorMode){
    /// Anchor Hosting is disabled. This is the value set in the default
    /// ::ArConfig.
    AR_CLOUD_ANCHOR_MODE_DISABLED = 0,
    /// Anchor Hosting is enabled. Setting this value and calling @c configure()
    /// will require that the application have the Android INTERNET permission.
    AR_CLOUD_ANCHOR_MODE_ENABLED = 1};

#undef AR_DEFINE_ENUM

#ifdef __cplusplus
extern "C" {
#endif

// Note: destroy methods do not take ArSession* to allow late destruction in
// finalizers of garbage-collected languages such as Java.

/// @addtogroup arcoreapk
/// @{

/// Determines if ARCore is supported on this device. This may initiate a query
/// with a remote service to determine if the device is compatible, in which
/// case it will return immediately with @c out_availability set to
/// #AR_AVAILABILITY_UNKNOWN_CHECKING.
///
/// For ARCore-required apps (as indicated by the <a
/// href="https://developers.google.com/ar/develop/c/enable-arcore#ar_required">manifest
/// meta-data</a>) this method will assume device compatibility and will always
/// immediately return one of #AR_AVAILABILITY_SUPPORTED_INSTALLED,
/// #AR_AVAILABILITY_SUPPORTED_APK_TOO_OLD, or
/// #AR_AVAILABILITY_SUPPORTED_NOT_INSTALLED.
///
/// Note: A result #AR_AVAILABILITY_SUPPORTED_INSTALLED only indicates presence
/// of a suitably versioned ARCore APK. Session creation may still fail if the
/// ARCore APK has been sideloaded onto an incompatible device.
///
/// May be called prior to ArSession_create().
///
/// @param[in] env The application's @c JNIEnv object
/// @param[in] application_context A @c jobject referencing the application's
///     Android @c Context.
/// @param[out] out_availability A pointer to an ArAvailability to receive
///     the result.
void ArCoreApk_checkAvailability(void *env,
                                 void *application_context,
                                 ArAvailability *out_availability);

/// Initiates installation of ARCore if needed. When your apllication launches
/// or enters an AR mode, it should call this method with @c
/// user_requested_install = 1.
///
/// If ARCore is installed and compatible, this function will set @c
/// out_install_status to #AR_INSTALL_STATUS_INSTALLED.
///
/// If ARCore is not currently installed or the installed version not
/// compatible, the function will set @c out_install_status to
/// #AR_INSTALL_STATUS_INSTALL_REQUESTED and return immediately. Your current
/// activity will then pause while the user is informed about the requierment of
/// ARCore and offered the opportunity to install it.
///
/// When your activity resumes, you should call this method again, this time
/// with @c user_requested_install = 0. This will either set
/// @c out_install_status to #AR_INSTALL_STATUS_INSTALLED or return an error
/// code indicating the reason that installation could not be completed.
///
/// ARCore-optional applications must ensure that ArCoreApk_checkAvailability()
/// returns one of the <tt>AR_AVAILABILITY_SUPPORTED_...</tt> values before
/// calling this method.
///
/// See <A
/// href="https://github.com/google-ar/arcore-android-sdk/tree/master/samples">
/// our sample code</A> for an example of how an ARCore-required application
/// should use this function.
///
/// May be called prior to ArSession_create().
///
/// For more control over the message displayed and ease of exiting the process,
/// see ArCoreApk_requestInstallCustom().
///
/// <b>Caution:</b> The value of <tt>*out_install_status</tt> should only be
/// considered when #AR_SUCCESS is returned.  Otherwise this value must be
/// ignored.
///
/// @param[in] env The application's @c JNIEnv object
/// @param[in] application_activity A @c jobject referencing the application's
///     current Android @c Activity.
/// @param[in] user_requested_install if set, override the previous installation
///     failure message and always show the installation interface.
/// @param[out] out_install_status A pointer to an ArInstallStatus to receive
///     the resulting install status, if successful.  Note: this value is only
///     valid with the return value is #AR_SUCCESS.
/// @return #AR_SUCCESS, or any of:
/// - #AR_ERROR_FATAL if an error occurs while checking for or requesting
///     installation
/// - #AR_UNAVAILABLE_DEVICE_NOT_COMPATIBLE if ARCore is not supported
///     on this device.
/// - #AR_UNAVAILABLE_USER_DECLINED_INSTALLATION if the user previously declined
///     installation.
ArStatus ArCoreApk_requestInstall(void *env,
                                  void *application_activity,
                                  bool user_requested_install,
                                  ArInstallStatus *out_install_status);

/// Initiates installation of ARCore if required, with configurable behavior.
///
/// This is a more flexible version of ArCoreApk_requestInstall() allowing the
/// application control over the initial informational dialog and ease of
/// exiting or cancelling the installation.
///
/// See ArCoreApk_requestInstall() for details of use and behavior.
///
/// May be called prior to ArSession_create().
///
/// @param[in] env The application's @c JNIEnv object
/// @param[in] application_activity A @c jobject referencing the application's
///     current Android @c Activity.
/// @param[in] user_requested_install if set, override the previous installation
///     failure message and always show the installation interface.
/// @param[in] install_behavior controls the presence of the cancel button at
///     the user education screen and if tapping outside the education screen or
///     install-in-progress screen causes them to dismiss.
/// @param[in] message_type controls the text of the of message displayed
///     before showing the install prompt, or disables display of this message.
/// @param[out] out_install_status A pointer to an ArInstallStatus to receive
///     the resulting install status, if successful.  Note: this value is only
///     valid with the return value is #AR_SUCCESS.
/// @return #AR_SUCCESS, or any of:
/// - #AR_ERROR_FATAL if an error occurs while checking for or requesting
///     installation
/// - #AR_UNAVAILABLE_DEVICE_NOT_COMPATIBLE if ARCore is not supported
///     on this device.
/// - #AR_UNAVAILABLE_USER_DECLINED_INSTALLATION if the user previously declined
///     installation.
ArStatus ArCoreApk_requestInstallCustom(void *env,
                                        void *application_activity,
                                        int32_t user_requested_install,
                                        ArInstallBehavior install_behavior,
                                        ArInstallUserMessageType message_type,
                                        ArInstallStatus *out_install_status);

/// @}
/// @addtogroup session
/// @{

/// Attempts to create a new ARCore session.
///
/// This is the entry point of ARCore.  This function MUST be the first ARCore
/// call made by an application.
///
/// @param[in]  env                 The application's @c JNIEnv object
/// @param[in]  application_context A @c jobject referencing the application's
///     Android @c Context
/// @param[out] out_session_pointer A pointer to an @c ArSession* to receive
///     the address of the newly allocated session.
/// @return #AR_SUCCESS or any of:
/// - #AR_UNAVAILABLE_ARCORE_NOT_INSTALLED
/// - #AR_UNAVAILABLE_DEVICE_NOT_COMPATIBLE
/// - #AR_UNAVAILABLE_APK_TOO_OLD
/// - #AR_UNAVAILABLE_SDK_TOO_OLD
/// - #AR_ERROR_CAMERA_PERMISSION_NOT_GRANTED
ArStatus ArSession_create(void *env,
                          void *application_context,
                          ArSession **out_session_pointer);

/// @}

// === ArConfig methods ===

/// @addtogroup config
/// @{

/// Creates a new configuration object and initializes it to a sensible default
/// configuration. Plane detection and lighting estimation are enabled, and
/// blocking update is selected. This configuration is guaranteed to be
/// supported on all devices that support ARCore.
void ArConfig_create(const ArSession *session, ArConfig **out_config);

/// Releases memory used by the provided configuration object.
void ArConfig_destroy(ArConfig *config);

/// Stores the currently configured lighting estimation mode into
/// @c *light_estimation_mode.
void ArConfig_getLightEstimationMode(
    const ArSession *session,
    const ArConfig *config,
    ArLightEstimationMode *light_estimation_mode);

/// Sets the lighting estimation mode that should be used. See
/// ::ArLightEstimationMode for available options.
void ArConfig_setLightEstimationMode(
    const ArSession *session,
    ArConfig *config,
    ArLightEstimationMode light_estimation_mode);

/// Stores the currently configured plane finding mode into
/// @c *plane_finding_mode.
void ArConfig_getPlaneFindingMode(const ArSession *session,
                                  const ArConfig *config,
                                  ArPlaneFindingMode *plane_finding_mode);

/// Sets the plane finding mode that should be used. See
/// ::ArPlaneFindingMode for available options.
void ArConfig_setPlaneFindingMode(const ArSession *session,
                                  ArConfig *config,
                                  ArPlaneFindingMode plane_finding_mode);

/// Stores the currently configured behavior of @ref ArSession_update() into
/// @c *update_mode.
void ArConfig_getUpdateMode(const ArSession *session,
                            const ArConfig *config,
                            ArUpdateMode *update_mode);

/// Sets the behavior of @ref ArSession_update(). See
/// ::ArUpdateMode for available options.
void ArConfig_setUpdateMode(const ArSession *session,
                            ArConfig *config,
                            ArUpdateMode update_mode);

/// Gets the current cloud anchor mode from the ::ArConfig.
void ArConfig_getCloudAnchorMode(const ArSession *session,
                                 const ArConfig *config,
                                 ArCloudAnchorMode *out_cloud_anchor_mode);

/// Sets the cloud configuration that should be used. See ::ArCloudAnchorMode
/// for available options.
void ArConfig_setCloudAnchorMode(const ArSession *session,
                                 ArConfig *config,
                                 ArCloudAnchorMode cloud_anchor_mode);

/// Sets the image database in the session configuration.
///
/// Any images in the currently active image database that have a
/// TRACKING/PAUSED state will immediately be set to the STOPPED state if a
/// different or null image database is set.
///
/// This function makes a copy of the image database.
void ArConfig_setAugmentedImageDatabase(
    const ArSession *session,
    ArConfig *config,
    const ArAugmentedImageDatabase *augmented_image_database);

/// Returns the image database from the session configuration.
///
/// This function returns a copy of the internally stored image database.
void ArConfig_getAugmentedImageDatabase(
    const ArSession *session,
    const ArConfig *config,
    ArAugmentedImageDatabase *out_augmented_image_database);

/// @}

// === ArSession methods ===

/// @addtogroup session
/// @{

/// Releases resources used by an ARCore session.
void ArSession_destroy(ArSession *session);

/// Before release 1.2.0: Checks if the provided configuration is usable on the
/// this device. If this method returns #AR_ERROR_UNSUPPORTED_CONFIGURATION,
/// calls to ArSession_configure(Config) with this configuration will fail.
///
/// This function now always returns true. See documentation for each
/// configuration entry to know which configuration options & combinations are
/// supported.
///
/// @param[in] session The ARCore session
/// @param[in] config  The configuration to test
/// @return #AR_SUCCESS or:
///  - #AR_ERROR_INVALID_ARGUMENT if any of the arguments are null.
/// @deprecated in release 1.2.0. Please refer to the release notes
/// (<a
/// href="https://github.com/google-ar/arcore-android-sdk/releases/tag/v1.2.0">release
/// notes 1.2.0</a>)
///
ArStatus ArSession_checkSupported(const ArSession *session,
                                  const ArConfig *config)
    AR_DEPRECATED(
        "deprecated in release 1.2.0. Please see function documentation");

/// Configures the session with the given config.
/// Note: a session is always initially configured with the default config.
/// This should be called if a configuration different than default is needed.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_FATAL
/// - #AR_ERROR_UNSUPPORTED_CONFIGURATION
ArStatus ArSession_configure(ArSession *session, const ArConfig *config);

/// Starts or resumes the ARCore Session.
///
/// Typically this should be called from <a
/// href="https://developer.android.com/reference/android/app/Activity.html#onResume()"
/// ><tt>Activity.onResume()</tt></a>.
///
/// @returns #AR_SUCCESS or any of:
/// - #AR_ERROR_FATAL
/// - #AR_ERROR_CAMERA_PERMISSION_NOT_GRANTED
/// - #AR_ERROR_CAMERA_NOT_AVAILABLE
ArStatus ArSession_resume(ArSession *session);

/// Pause the current session. This method will stop the camera feed and release
/// resources. The session can be restarted again by calling ArSession_resume().
///
/// Typically this should be called from <a
/// href="https://developer.android.com/reference/android/app/Activity.html#onPause()"
/// ><tt>Activity.onPause()</tt></a>.
///
/// @returns #AR_SUCCESS or any of:
/// - #AR_ERROR_FATAL
ArStatus ArSession_pause(ArSession *session);

/// Sets the OpenGL texture name (id) that will allow GPU access to the camera
/// image. The provided ID should have been created with @c glGenTextures(). The
/// resulting texture must be bound to the @c GL_TEXTURE_EXTERNAL_OES target for
/// use. Shaders accessing this texture must use a @c samplerExternalOES
/// sampler. See sample code for an example.
void ArSession_setCameraTextureName(ArSession *session, uint32_t texture_id);

/// Sets the aspect ratio, coordinate scaling, and display rotation. This data
/// is used by UV conversion, projection matrix generation, and hit test logic.
///
/// Note: this function doesn't fail. If given invalid input, it logs a error
/// and doesn't apply the changes.
///
/// @param[in] session   The ARCore session
/// @param[in] rotation  Display rotation specified by @c android.view.Surface
///     constants: @c ROTATION_0, @c ROTATION_90, @c ROTATION_180 and
///     @c ROTATION_270
/// @param[in] width     Width of the view, in pixels
/// @param[in] height    Height of the view, in pixels
void ArSession_setDisplayGeometry(ArSession *session,
                                  int32_t rotation,
                                  int32_t width,
                                  int32_t height);

/// Updates the state of the ARCore system. This includes: receiving a new
/// camera frame, updating the location of the device, updating the location of
/// tracking anchors, updating detected planes, etc.
///
/// This call may cause off-screen OpenGL activity. Because of this, to avoid
/// unnecessary frame buffer flushes and reloads, this call should not be made
/// in the middle of rendering a frame or offscreen buffer.
///
/// This call may update the pose of all created anchors and detected planes.
/// The set of updated objects is accessible through
/// ArFrame_getUpdatedTrackables().
///
/// @c update() in blocking mode (see ::ArUpdateMode) will wait until a
/// new camera image is available, or until the built-in timeout
/// (currently 66ms) is reached.
/// If the camera image does not arrive by the built-in timeout, then
/// @c update() will return the most recent ::ArFrame object. For some
/// applications it may be important to know if a new frame was actually
/// obtained (for example, to avoid redrawing if the camera did not produce a
/// new frame). To do that, compare the current frame's timestamp, obtained via
/// @c ArFrame_getTimestamp, with the previously recorded frame timestamp. If
/// they are different, this is a new frame.
///
/// @param[in]    session   The ARCore session
/// @param[inout] out_frame The Frame object to populate with the updated world
///     state.  This frame must have been previously created using
///     ArFrame_create().  The same ArFrame instance may be used when calling
///     this repeatedly.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_FATAL
/// - #AR_ERROR_SESSION_PAUSED
/// - #AR_ERROR_TEXTURE_NOT_SET
/// - #AR_ERROR_MISSING_GL_CONTEXT
/// - #AR_ERROR_CAMERA_NOT_AVAILABLE - camera was removed during runtime.
ArStatus ArSession_update(ArSession *session, ArFrame *out_frame);

/// Defines a tracked location in the physical world.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_NOT_TRACKING
/// - #AR_ERROR_SESSION_PAUSED
/// - #AR_ERROR_RESOURCE_EXHAUSTED
ArStatus ArSession_acquireNewAnchor(ArSession *session,
                                    const ArPose *pose,
                                    ArAnchor **out_anchor);

/// Returns all known anchors, including those not currently tracked. Anchors
/// forgotten by ARCore due to a call to ArAnchor_detach() or entering the
/// #AR_TRACKING_STATE_STOPPED state will not be included.
///
/// @param[in]    session         The ARCore session
/// @param[inout] out_anchor_list The list to fill.  This list must have already
///     been allocated with ArAnchorList_create().  If previously used, the list
///     will first be cleared.
void ArSession_getAllAnchors(const ArSession *session,
                             ArAnchorList *out_anchor_list);

/// Returns the list of all known @ref trackable "trackables".  This includes
/// ::ArPlane objects if plane detection is enabled, as well as ::ArPoint
/// objects created as a side effect of calls to ArSession_acquireNewAnchor() or
/// ArFrame_hitTest().
///
/// @param[in]    session            The ARCore session
/// @param[in]    filter_type        The type(s) of trackables to return.  See
///     ::ArTrackableType for legal values.
/// @param[inout] out_trackable_list The list to fill.  This list must have
///     already been allocated with ArTrackableList_create().  If previously
///     used, the list will first be cleared.
void ArSession_getAllTrackables(const ArSession *session,
                                ArTrackableType filter_type,
                                ArTrackableList *out_trackable_list);

/// This will create a new cloud anchor using pose and other metadata from
/// @c anchor.
///
/// If the function returns #AR_SUCCESS, the cloud state of @c out_cloud_anchor
/// will be set to #AR_CLOUD_ANCHOR_STATE_TASK_IN_PROGRESS and the initial pose
/// will be set to the pose of @c anchor. However, the new @c out_cloud_anchor
/// is completely independent of @c anchor, and the poses may diverge over time.
/// If the return value of this function is not #AR_SUCCESS, then
/// @c out_cloud_anchor will be set to null.
///
/// @param[in]    session          The ARCore session
/// @param[in]    anchor           The anchor to be hosted
/// @param[inout] out_cloud_anchor The new cloud anchor
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_NOT_TRACKING
/// - #AR_ERROR_SESSION_PAUSED
/// - #AR_ERROR_CLOUD_ANCHORS_NOT_CONFIGURED
/// - #AR_ERROR_RESOURCE_EXHAUSTED
/// - #AR_ERROR_ANCHOR_NOT_SUPPORTED_FOR_HOSTING
ArStatus ArSession_hostAndAcquireNewCloudAnchor(ArSession *session,
                                                const ArAnchor *anchor,
                                                ArAnchor **out_cloud_anchor);

/// This will create a new cloud anchor, and schedule a resolving task to
/// resolve the anchor's pose using the given cloud anchor ID.
///
/// If this function returns #AR_SUCCESS, the cloud state of @c out_cloud_anchor
/// will be #AR_CLOUD_STATE_TASK_IN_PROGRESS, and its tracking state will be
/// #AR_TRACKING_STATE_PAUSED. This anchor will never start tracking until its
/// pose has been successfully resolved. If the resolving task ends in an error,
/// the tracking state will be set to #AR_TRACKING_STATE_STOPPED. If the return
/// value is not #AR_SUCCESS, then @c out_cloud_anchor will be set to null.
///
/// @param[in]    session          The ARCore session
/// @param[in]    cloud_anchor_id  The cloud ID of the anchor to be resolved
/// @param[inout] out_cloud_anchor The new cloud anchor
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_NOT_TRACKING
/// - #AR_ERROR_SESSION_PAUSED
/// - #AR_ERROR_CLOUD_ANCHORS_NOT_CONFIGURED
/// - #AR_ERROR_RESOURCE_EXHAUSTED
ArStatus ArSession_resolveAndAcquireNewCloudAnchor(ArSession *session,
                                                   const char *cloud_anchor_id,
                                                   ArAnchor **out_cloud_anchor);

/// @}

// === ArPose methods ===

/// @addtogroup pose
/// @{

/// Allocates and initializes a new pose object.  @c pose_raw points to an array
/// of 7 floats, describing the rotation (quaternion) and translation of the
/// pose in the same order as the first 7 elements of the Android
/// @c Sensor.TYPE_POSE_6DOF values documented on <a
/// href="https://developer.android.com/reference/android/hardware/SensorEvent.html#values"
/// >@c SensorEvent.values() </a>
///
/// The order of the values is: qx, qy, qz, qw, tx, ty, tz.
///
/// If @c pose_raw is null, initializes with the identity pose.
void ArPose_create(const ArSession *session,
                   const float *pose_raw,
                   ArPose **out_pose);

/// Releases memory used by a pose object.
void ArPose_destroy(ArPose *pose);

/// Extracts the quaternion rotation and translation from a pose object.
/// @param[in]  session       The ARCore session
/// @param[in]  pose          The pose to extract
/// @param[out] out_pose_raw  Pointer to an array of 7 floats, to be filled with
///     the quaternion rotation and translation as described in ArPose_create().
void ArPose_getPoseRaw(const ArSession *session,
                       const ArPose *pose,
                       float *out_pose_raw);

/// Converts a pose into a 4x4 transformation matrix.
/// @param[in]  session                  The ARCore session
/// @param[in]  pose                     The pose to convert
/// @param[out] out_matrix_col_major_4x4 Pointer to an array of 16 floats, to be
///     filled with a column-major homogenous transformation matrix, as used by
///     OpenGL.
void ArPose_getMatrix(const ArSession *session,
                      const ArPose *pose,
                      float *out_matrix_col_major_4x4);

/// @}

// === ArCamera methods ===

/// @addtogroup camera
/// @{

/// Sets @c out_pose to the pose of the user's device in the world coordinate
/// space at the time of capture of the current camera texture. The position and
/// orientation of the pose follow the device's physical camera (they are not
/// affected by display orientation) and uses OpenGL camera conventions (+X
/// right, +Y up, -Z in the direction the camera is looking).
///
/// Note: This pose is only useful when ArCamera_getTrackingState() returns
/// #AR_TRACKING_STATE_TRACKING and otherwise should not be used.
///
/// @param[in]    session  The ARCore session
/// @param[in]    camera   The session's camera (retrieved from any frame).
/// @param[inout] out_pose An already-allocated ArPose object into which the
///     pose will be stored.
void ArCamera_getPose(const ArSession *session,
                      const ArCamera *camera,
                      ArPose *out_pose);

/// Sets @c out_pose to the pose of the user's device in the world coordinate
/// space at the time of capture of the current camera texture. The position of
/// the pose is located at the device's camera, while the orientation
/// approximately matches the orientation of the display (considering display
/// rotation), using OpenGL camera conventions (+X right, +Y up, -Z in the
/// direction the camera is looking).
///
/// Note: This pose is only useful when ArCamera_getTrackingState() returns
/// #AR_TRACKING_STATE_TRACKING and otherwise should not be used.
///
/// See also: ArCamera_getViewMatrix()
///
/// @param[in]    session  The ARCore session
/// @param[in]    camera   The session's camera (retrieved from any frame).
/// @param[inout] out_pose An already-allocated ArPose object into which the
///     pose will be stored.
void ArCamera_getDisplayOrientedPose(const ArSession *session,
                                     const ArCamera *camera,
                                     ArPose *out_pose);

/// Returns the view matrix for the camera for this frame. This matrix performs
/// the inverse transfrom as the pose provided by
/// ArCamera_getDisplayOrientedPose().
///
/// @param[in]    session           The ARCore session
/// @param[in]    camera            The session's camera.
/// @param[inout] out_col_major_4x4 Pointer to an array of 16 floats, to be
///     filled with a column-major homogenous transformation matrix, as used by
///     OpenGL.
void ArCamera_getViewMatrix(const ArSession *session,
                            const ArCamera *camera,
                            float *out_col_major_4x4);

/// Gets the current state of the pose of this camera. If this state is anything
/// other than #AR_TRACKING_STATE_TRACKING the Camera's pose should not be
/// considered useful.
void ArCamera_getTrackingState(const ArSession *session,
                               const ArCamera *camera,
                               ArTrackingState *out_tracking_state);

/// Computes a projection matrix for rendering virtual content on top of the
/// camera image. Note that the projection matrix reflects the current display
/// geometry and display rotation.
///
/// @param[in]    session            The ARCore session
/// @param[in]    camera             The session's camera.
/// @param[in]    near               Specifies the near clip plane, in meters
/// @param[in]    far                Specifies the far clip plane, in meters
/// @param[inout] dest_col_major_4x4 Pointer to an array of 16 floats, to
///     be filled with a column-major homogenous transformation matrix, as used
///     by OpenGL.
void ArCamera_getProjectionMatrix(const ArSession *session,
                                  const ArCamera *camera,
                                  float near,
                                  float far,
                                  float *dest_col_major_4x4);

/// Releases a reference to the camera.  This must match a call to
/// ArFrame_acquireCamera().
///
/// This method may safely be called with @c nullptr - it will do nothing.
void ArCamera_release(ArCamera *camera);

/// @}

// === ArFrame methods ===

/// @addtogroup frame
/// @{

/// Allocates a new ArFrame object, storing the pointer into @c *out_frame.
///
/// Note: the same ArFrame can be used repeatedly when calling ArSession_update.
void ArFrame_create(const ArSession *session, ArFrame **out_frame);

/// Releases an ArFrame and any references it holds.
void ArFrame_destroy(ArFrame *frame);

/// Checks if the display rotation or viewport geometry changed since the
/// previous call to ArSession_update(). The application should re-query
/// ArCamera_getProjectionMatrix() and ArFrame_transformDisplayUvCoords()
/// whenever this emits non-zero.
void ArFrame_getDisplayGeometryChanged(const ArSession *session,
                                       const ArFrame *frame,
                                       int32_t *out_geometry_changed);

/// Returns the timestamp in nanoseconds when this image was captured. This can
/// be used to detect dropped frames or measure the camera frame rate. The time
/// base of this value is specifically <b>not</b> defined, but it is likely
/// similar to <tt>clock_gettime(CLOCK_BOOTTIME)</tt>.
void ArFrame_getTimestamp(const ArSession *session,
                          const ArFrame *frame,
                          int64_t *out_timestamp_ns);

/// Transform the given texture coordinates to correctly show the background
/// image. This will account for the display rotation, and any additional
/// required adjustment. For performance, this function should be called only if
/// ArFrame_hasDisplayGeometryChanged() emits true.
///
/// @param[in]    session      The ARCore session
/// @param[in]    frame        The current frame.
/// @param[in]    num_elements The number of floats to transform.  Must be
///     a multiple of 2.  @c uvs_in and @c uvs_out must point to arrays of at
///     least this many floats.
/// @param[in]    uvs_in       Input UV coordinates in normalized screen space.
/// @param[inout] uvs_out      Output UV coordinates in texture coordinates.
void ArFrame_transformDisplayUvCoords(const ArSession *session,
                                      const ArFrame *frame,
                                      int32_t num_elements,
                                      const float *uvs_in,
                                      float *uvs_out);

/// Performs a ray cast from the user's device in the direction of the given
/// location in the camera view. Intersections with detected scene geometry are
/// returned, sorted by distance from the device; the nearest intersection is
/// returned first.
///
/// Note: Significant geometric leeway is given when returning hit results. For
/// example, a plane hit may be generated if the ray came close, but did not
/// actually hit within the plane extents or plane bounds
/// (ArPlane_isPoseInExtents() and ArPlane_isPoseInPolygon() can be used to
/// determine these cases). A point (point cloud) hit is generated when a point
/// is roughly within one finger-width of the provided screen coordinates.
///
/// The resulting list is ordered by distance, with the nearest hit first
///
/// Note: If not tracking, the hit_result_list will be empty. <br>
/// Note: If called on an old frame (not the latest produced by
///     ArSession_update() the hit_result_list will be empty).
///
/// @param[in]    session         The ARCore session.
/// @param[in]    frame           The current frame.
/// @param[in]    pixel_x         Logical X position within the view, as from an
///     Android UI event.
/// @param[in]    pixel_y         Logical X position within the view.
/// @param[inout] hit_result_list The list to fill.  This list must have been
///     previously allocated using ArHitResultList_create().  If the list has
///     been previously used, it will first be cleared.
void ArFrame_hitTest(const ArSession *session,
                     const ArFrame *frame,
                     float pixel_x,
                     float pixel_y,
                     ArHitResultList *hit_result_list);

/// Gets the current ambient light estimate, if light estimation was enabled.
///
/// @param[in]    session            The ARCore session.
/// @param[in]    frame              The current frame.
/// @param[inout] out_light_estimate The light estimate to fill.  This object
///    must have been previously created with ArLightEstimate_create().
void ArFrame_getLightEstimate(const ArSession *session,
                              const ArFrame *frame,
                              ArLightEstimate *out_light_estimate);

/// Acquires the current set of estimated 3d points attached to real-world
/// geometry. A matching call to PointCloud_release() must be made when the
/// application is done accessing the point cloud.
///
/// Note: This information is for visualization and debugging purposes only. Its
/// characteristics and format are subject to change in subsequent versions of
/// the API.
///
/// @param[in]  session         The ARCore session.
/// @param[in]  frame           The current frame.
/// @param[out] out_point_cloud Pointer to an @c ArPointCloud* receive the
///     address of the point cloud.
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_DEADLINE_EXCEEDED if @c frame is not the latest frame from
///   by ArSession_update().
/// - #AR_ERROR_RESOURCE_EXHAUSTED if too many point clouds are currently held.
ArStatus ArFrame_acquirePointCloud(const ArSession *session,
                                   const ArFrame *frame,
                                   ArPointCloud **out_point_cloud);

/// Returns the camera object for the session. Note that this Camera instance is
/// long-lived so the same instance is returned regardless of the frame object
/// this method was called on.
void ArFrame_acquireCamera(const ArSession *session,
                           const ArFrame *frame,
                           ArCamera **out_camera);

/// Gets the camera metadata for the current camera image.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_DEADLINE_EXCEEDED if @c frame is not the latest frame from
///   by ArSession_update().
/// - #AR_ERROR_RESOURCE_EXHAUSTED if too many metadata objects are currently
///   held.
/// - #AR_ERROR_NOT_YET_AVAILABLE if the camera failed to produce metadata for
///   the given frame. Note: this will commonly happen for few frames right
///   after @c ArSession_resume() due to the camera stack bringup.
ArStatus ArFrame_acquireImageMetadata(const ArSession *session,
                                      const ArFrame *frame,
                                      ArImageMetadata **out_metadata);

/// Gets the set of anchors that were changed by the ArSession_update() that
/// produced this Frame.
///
/// @param[in]    session            The ARCore session
/// @param[in]    frame              The current frame.
/// @param[inout] out_anchor_list The list to fill.  This list must have
///     already been allocated with ArAnchorList_create().  If previously
///     used, the list will first be cleared.
void ArFrame_getUpdatedAnchors(const ArSession *session,
                               const ArFrame *frame,
                               ArAnchorList *out_anchor_list);

/// Gets the set of trackables of a particular type that were changed by the
/// ArSession_update() call that produced this Frame.
///
/// @param[in]    session            The ARCore session
/// @param[in]    frame              The current frame.
/// @param[in]    filter_type        The type(s) of trackables to return.  See
///     ::ArTrackableType for legal values.
/// @param[inout] out_trackable_list The list to fill.  This list must have
///     already been allocated with ArTrackableList_create().  If previously
///     used, the list will first be cleared.
void ArFrame_getUpdatedTrackables(const ArSession *session,
                                  const ArFrame *frame,
                                  ArTrackableType filter_type,
                                  ArTrackableList *out_trackable_list);
/// @}

// === ArPointCloud methods ===

/// @addtogroup pointcloud
/// @{

/// Retrieves the number of points in the point cloud.
///
void ArPointCloud_getNumberOfPoints(const ArSession *session,
                                    const ArPointCloud *point_cloud,
                                    int32_t *out_number_of_points);

/// Retrieves a pointer to the point cloud data.
///
/// Each point is represented by four consecutive values in the array; first the
/// X, Y, Z position coordinates, followed by a confidence value. This is the
/// same format as described in <a
/// href="https://developer.android.com/reference/android/graphics/ImageFormat.html#DEPTH_POINT_CLOUD"
/// >DEPTH_POINT_CLOUD</a>.
///
/// The pointer returned by this function is valid until ArPointCloud_release()
/// is called. The application must copy the data if they wish to retain it for
/// longer. The points are in world coordinates consistent with the frame it was
/// obtained from. If the number of points is zero, then the value of
/// @c *out_point_cloud_data should is undefined.
void ArPointCloud_getData(const ArSession *session,
                          const ArPointCloud *point_cloud,
                          const float **out_point_cloud_data);

/// Returns the timestamp in nanoseconds when this point cloud was observed.
/// This timestamp uses the same time base as ArFrame_getTimestamp().
void ArPointCloud_getTimestamp(const ArSession *session,
                               const ArPointCloud *point_cloud,
                               int64_t *out_timestamp_ns);

/// Releases a reference to the point cloud.  This must match a call to
/// ArFrame_acquirePointCloud().
///
/// This method may safely be called with @c nullptr - it will do nothing.
void ArPointCloud_release(ArPointCloud *point_cloud);

/// @}

// === Image Metadata methods ===

/// @addtogroup image
/// @{

/// Retrieves the capture metadata for the current camera image.
///
/// @c ACameraMetadata is a struct in Android NDK. Include NdkCameraMetadata.h
/// to use this type.
///
/// Note: that the ACameraMetadata returned from this function will be invalid
/// after its ArImageMetadata object is released.
void ArImageMetadata_getNdkCameraMetadata(
    const ArSession *session,
    const ArImageMetadata *image_metadata,
    const ACameraMetadata **out_ndk_metadata);

/// Releases a reference to the metadata.  This must match a call to
/// ArFrame_acquireImageMetadata().
///
/// This method may safely be called with @c nullptr - it will do nothing.
void ArImageMetadata_release(ArImageMetadata *metadata);

// === CPU Image Access types and methods ===
/// Gets the image of the tracking camera relative to the input session and
/// frame.
/// Return values:
/// @returns #AR_SUCCESS or any of:
/// - #AR_ERROR_INVALID_ARGUMENT - one more input arguments are invalid.
/// - #AR_ERROR_DEADLINE_EXCEEDED - the input frame is not the current frame.
/// - #AR_ERROR_RESOURCE_EXHAUSTED - the caller app has exceeded maximum number
///   of images that it can hold without releasing.
/// - #AR_ERROR_NOT_YET_AVAILABLE - image with the timestamp of the input frame
///   was not found within a bounded amount of time, or the camera failed to
///   produce the image
ArStatus ArFrame_acquireCameraImage(ArSession *session,
                                    ArFrame *frame,
                                    ArImage **out_image);

/// Converts an ArImage object to an Android NDK AImage object.
void ArImage_getNdkImage(const ArImage *image, const AImage **out_ndk_image);

/// Releases an instance of ArImage returned by ArFrame_acquireCameraImage().
void ArImage_release(ArImage *image);
/// @}

// === ArLightEstimate methods ===

/// @addtogroup light
/// @{

/// Allocates a light estimate object.
void ArLightEstimate_create(const ArSession *session,
                            ArLightEstimate **out_light_estimate);

/// Releases the provided light estimate object.
void ArLightEstimate_destroy(ArLightEstimate *light_estimate);

/// Retrieves the validity state of a light estimate.  If the resulting value of
/// @c *out_light_estimate_state is not #AR_LIGHT_ESTIMATE_STATE_VALID, the
/// estimate should not be used for rendering.
void ArLightEstimate_getState(const ArSession *session,
                              const ArLightEstimate *light_estimate,
                              ArLightEstimateState *out_light_estimate_state);

/// Retrieves the pixel intensity, in gamma space, of the current camera view.
/// Values are in the range (0.0, 1.0), with zero being black and one being
/// white.
/// If rendering in gamma space, divide this value by 0.466, which is middle
/// gray in gamma space, and multiply against the final calculated color after
/// rendering.
/// If rendering in linear space, first convert this value to linear space by
/// rising to the power 2.2. Normalize the result by dividing it by 0.18 which
/// is middle gray in linear space. Then multiply by the final calculated color
/// after rendering.
void ArLightEstimate_getPixelIntensity(const ArSession *session,
                                       const ArLightEstimate *light_estimate,
                                       float *out_pixel_intensity);

/// Gets the color correction values that are uploaded to the fragment shader.
/// Use the RGB scale factors (components 0-2) to match the color of the light
/// in the scene. Use the pixel intensity (component 3) to match the intensity
/// of the light in the scene.
///
/// `out_color_correction_4` components are:
///   - `[0]` Red channel scale factor.
///   - `[1]` Green channel scale factor.
///   - `[2]` Blue channel scale factor.
///   - `[3]` Pixel intensity. This is the same value as the one return from
///       ArLightEstimate_getPixelIntensity().
///
///  The RGB scale factors can be used independently from the pixel intensity
///  value. They are put together for the convenience of only having to upload
///  one float4 to the fragment shader.
///
///  The RGB scale factors are not intended to brighten nor dim the scene.  They
///  are only to shift the color of the virtual object towards the color of the
///  light; not intensity of the light. The pixel intensity is used to match the
///  intensity of the light in the scene.
///
///  Color correction values are reported in gamma space.
///  If rendering in gamma space, component-wise multiply them against the final
///  calculated color after rendering.
///  If rendering in linear space, first convert the values to linear space by
///  rising to the power 2.2. Then component-wise multiply against the final
///  calculated color after rendering.
void ArLightEstimate_getColorCorrection(const ArSession *session,
                                        const ArLightEstimate *light_estimate,
                                        float *out_color_correction_4);

/// @}

// === ArAnchorList methods ===

/// @addtogroup anchor
/// @{

/// Creates an anchor list object.
void ArAnchorList_create(const ArSession *session,
                         ArAnchorList **out_anchor_list);

/// Releases the memory used by an anchor list object, along with all the anchor
/// references it holds.
void ArAnchorList_destroy(ArAnchorList *anchor_list);

/// Retrieves the number of anchors in this list.
void ArAnchorList_getSize(const ArSession *session,
                          const ArAnchorList *anchor_list,
                          int32_t *out_size);

/// Acquires a reference to an indexed entry in the list.  This call must
/// eventually be matched with a call to ArAnchor_release().
void ArAnchorList_acquireItem(const ArSession *session,
                              const ArAnchorList *anchor_list,
                              int32_t index,
                              ArAnchor **out_anchor);

// === ArAnchor methods ===

/// Retrieves the pose of the anchor in the world coordinate space. This pose
/// produced by this call may change each time ArSession_update() is called.
/// This pose should only be used for rendering if ArAnchor_getTrackingState()
/// returns #AR_TRACKING_STATE_TRACKING.
///
/// @param[in]    session  The ARCore session.
/// @param[in]    anchor   The anchor to retrieve the pose of.
/// @param[inout] out_pose An already-allocated ArPose object into which the
///     pose will be stored.
void ArAnchor_getPose(const ArSession *session,
                      const ArAnchor *anchor,
                      ArPose *out_pose);

/// Retrieves the current state of the pose of this anchor.
void ArAnchor_getTrackingState(const ArSession *session,
                               const ArAnchor *anchor,
                               ArTrackingState *out_tracking_state);

/// Tells ARCore to stop tracking and forget this anchor.  This call does not
/// release the reference to the anchor - that must be done separately using
/// ArAnchor_release().
void ArAnchor_detach(ArSession *session, ArAnchor *anchor);

/// Releases a reference to an anchor. This does not mean that the anchor will
/// stop tracking, as it will be obtainable from e.g. ArSession_getAllAnchors()
/// if any other references exist.
///
/// This method may safely be called with @c nullptr - it will do nothing.
void ArAnchor_release(ArAnchor *anchor);

/// Acquires the cloud anchor ID of the anchor. The ID acquired is an ASCII
/// null-terminated string. The acquired ID must be released after use by the
/// @c ArString_release function. For anchors with cloud state
/// #AR_CLOUD_ANCHOR_STATE_NONE or #AR_CLOUD_ANCHOR_STATE_TASK_IN_PROGRESS, this
/// will always be an empty string.
///
/// @param[in]    session             The ARCore session.
/// @param[in]    anchor              The anchor to retrieve the cloud ID of.
/// @param[inout] out_cloud_anchor_id A pointer to the acquired ID string.
void ArAnchor_acquireCloudAnchorId(ArSession *session,
                                   ArAnchor *anchor,
                                   char **out_cloud_anchor_id);

/// Gets the current cloud anchor state of the anchor. This state is guaranteed
/// not to change until update() is called.
///
/// @param[in]    session   The ARCore session.
/// @param[in]    anchor    The anchor to retrieve the cloud state of.
/// @param[inout] out_state The current cloud state of the anchor.
void ArAnchor_getCloudAnchorState(const ArSession *session,
                                  const ArAnchor *anchor,
                                  ArCloudAnchorState *out_state);

/// @}

// === ArTrackableList methods ===

/// @addtogroup trackable
/// @{

/// Creates a trackable list object.
void ArTrackableList_create(const ArSession *session,
                            ArTrackableList **out_trackable_list);

/// Releases the memory used by a trackable list object, along with all the
/// anchor references it holds.
void ArTrackableList_destroy(ArTrackableList *trackable_list);

/// Retrieves the number of trackables in this list.
void ArTrackableList_getSize(const ArSession *session,
                             const ArTrackableList *trackable_list,
                             int32_t *out_size);

/// Acquires a reference to an indexed entry in the list.  This call must
/// eventually be matched with a call to ArTrackable_release().
void ArTrackableList_acquireItem(const ArSession *session,
                                 const ArTrackableList *trackable_list,
                                 int32_t index,
                                 ArTrackable **out_trackable);

// === ArTrackable methods ===

/// Releases a reference to a trackable. This does not mean that the trackable
/// will necessarily stop tracking. The same trackable may still be included in
/// from other calls, for example ArSession_getAllTrackables().
///
/// This method may safely be called with @c nullptr - it will do nothing.
void ArTrackable_release(ArTrackable *trackable);

/// Retrieves the type of the trackable.  See ::ArTrackableType for valid types.
void ArTrackable_getType(const ArSession *session,
                         const ArTrackable *trackable,
                         ArTrackableType *out_trackable_type);

/// Retrieves the current state of ARCore's knowledge of the pose of this
/// trackable.
void ArTrackable_getTrackingState(const ArSession *session,
                                  const ArTrackable *trackable,
                                  ArTrackingState *out_tracking_state);

/// Creates an Anchor at the given pose in the world coordinate space, attached
/// to this Trackable, and acquires a reference to it. The type of Trackable
/// will determine the semantics of attachment and how the Anchor's pose will be
/// updated to maintain this relationship. Note that the relative offset between
/// the pose of multiple Anchors attached to a Trackable may adjust slightly
/// over time as ARCore updates its model of the world.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_NOT_TRACKING if the trackable's tracking state was not
///   #AR_TRACKING_STATE_TRACKING
/// - #AR_ERROR_SESSION_PAUSED if the session was paused
/// - #AR_ERROR_RESOURCE_EXHAUSTED if too many anchors exist
ArStatus ArTrackable_acquireNewAnchor(ArSession *session,
                                      ArTrackable *trackable,
                                      ArPose *pose,
                                      ArAnchor **out_anchor);

/// Gets the set of anchors attached to this trackable.
///
/// @param[in]    session         The ARCore session
/// @param[in]    trackable       The trackable to query the anchors of.
/// @param[inout] out_anchor_list The list to fill.  This list must have
///     already been allocated with ArAnchorList_create().  If previously
///     used, the list will first be cleared.
void ArTrackable_getAnchors(const ArSession *session,
                            const ArTrackable *trackable,
                            ArAnchorList *out_anchor_list);

/// @}

// === ArPlane methods ===

/// @addtogroup plane
/// @{

/// Acquires a reference to the plane subsuming this plane.
///
/// Two or more planes may be automatically merged into a single parent plane,
/// resulting in this method acquiring the parent plane when called with each
/// child plane. A subsumed plane becomes identical to the parent plane, and
/// will continue behaving as if it were independently tracked, for example
/// being included in the output of ArFrame_getUpdatedTrackables().
///
/// In cases where a subsuming plane is itself subsumed, this function
/// will always return the topmost non-subsumed plane.
///
/// Note: this function will set @c *out_subsumed_by to NULL if the plane is not
/// subsumed.
void ArPlane_acquireSubsumedBy(const ArSession *session,
                               const ArPlane *plane,
                               ArPlane **out_subsumed_by);

/// Retrieves the type (orientation) of the plane.  See ::ArPlaneType.
void ArPlane_getType(const ArSession *session,
                     const ArPlane *plane,
                     ArPlaneType *out_plane_type);

/// Returns the pose of the center of the detected plane. The pose's transformed
/// +Y axis will be point normal out of the plane, with the +X and +Z axes
/// orienting the extents of the bounding rectangle.
///
/// @param[in]    session  The ARCore session.
/// @param[in]    plane    The plane for which to retrieve center pose.
/// @param[inout] out_pose An already-allocated ArPose object into which the
///     pose will be stored.
void ArPlane_getCenterPose(const ArSession *session,
                           const ArPlane *plane,
                           ArPose *out_pose);

/// Retrieves the length of this plane's bounding rectangle measured along the
/// local X-axis of the coordinate space defined by the output of
/// ArPlane_getCenterPose().
void ArPlane_getExtentX(const ArSession *session,
                        const ArPlane *plane,
                        float *out_extent_x);

/// Retrieves the length of this plane's bounding rectangle measured along the
/// local Z-axis of the coordinate space defined by the output of
/// ArPlane_getCenterPose().
void ArPlane_getExtentZ(const ArSession *session,
                        const ArPlane *plane,
                        float *out_extent_z);

/// Retrieves the number of elements (not vertices) in the boundary polygon.
/// The number of vertices is 1/2 this size.
void ArPlane_getPolygonSize(const ArSession *session,
                            const ArPlane *plane,
                            int32_t *out_polygon_size);

/// Returns the 2D vertices of a convex polygon approximating the detected
/// plane, in the form <tt>[x1, z1, x2, z2, ...]</tt>. These X-Z values are in
/// the plane's local x-z plane (y=0) and must be transformed by the pose
/// (ArPlane_getCenterPose()) to get the boundary in world coordinates.
///
/// @param[in]    session        The ARCore session.
/// @param[in]    plane          The plane to retrieve the polygon from.
/// @param[inout] out_polygon_xz A pointer to an array of floats.  The length of
///     this array must be at least that reported by ArPlane_getPolygonSize().
void ArPlane_getPolygon(const ArSession *session,
                        const ArPlane *plane,
                        float *out_polygon_xz);

/// Sets @c *out_pose_in_extents to non-zero if the given pose (usually obtained
/// from a HitResult) is in the plane's rectangular extents.
void ArPlane_isPoseInExtents(const ArSession *session,
                             const ArPlane *plane,
                             const ArPose *pose,
                             int32_t *out_pose_in_extents);

/// Sets @c *out_pose_in_extents to non-zero if the given pose (usually obtained
/// from a HitResult) is in the plane's polygon.
void ArPlane_isPoseInPolygon(const ArSession *session,
                             const ArPlane *plane,
                             const ArPose *pose,
                             int32_t *out_pose_in_polygon);

/// @}

// === ArPoint methods ===

/// @addtogroup point
/// @{

/// Returns the pose of the point.
/// If ArPoint_getOrientationMode() returns ESTIMATED_SURFACE_NORMAL, the
/// orientation will follow the behavior described in ArHitResult_getHitPose().
/// If ArPoint_getOrientationMode() returns INITIALIZED_TO_IDENTITY, then
/// returns an orientation that is identity or close to identity.
/// @param[in]    session  The ARCore session.
/// @param[in]    point    The point to retrieve the pose of.
/// @param[inout] out_pose An already-allocated ArPose object into which the
/// pose will be stored.
void ArPoint_getPose(const ArSession *session,
                     const ArPoint *point,
                     ArPose *out_pose);

/// Returns the OrientationMode of the point. For @c Point objects created by
/// ArFrame_hitTest().
/// If OrientationMode is ESTIMATED_SURFACE_NORMAL, then normal of the surface
/// centered around the ArPoint was estimated succesfully.
///
/// @param[in]    session              The ARCore session.
/// @param[in]    point                The point to retrieve the pose of.
/// @param[inout] out_orientation_mode OrientationMode output result for the
///     the point.
void ArPoint_getOrientationMode(const ArSession *session,
                                const ArPoint *point,
                                ArPointOrientationMode *out_orientation_mode);

/// @}

// === ArAugmentedImage methods ===

/// @addtogroup augmented_image
/// @{

/// Returns the pose of the center of the detected image. The pose's
/// transformed +Y axis will be point normal out of the image.
///
/// If the tracking state is PAUSED/STOPPED, this returns the pose when the
/// image state was last TRACKING, or the identity pose if the image state has
/// never been TRACKING.
void ArAugmentedImage_getCenterPose(const ArSession *session,
                                    const ArAugmentedImage *augmented_image,
                                    ArPose *out_pose);

/// Retrieves the estimated width, in metres, of the corresponding physical
/// image, as measured along the local X-axis of the coordinate space with
/// origin and axes as defined by ArAugmentedImage_getCenterPose().
///
/// ARCore will attempt to estimate the physical image's width and continuously
/// update this estimate based on its understanding of the world. If the
/// optional physical size is specified in the image database, this estimation
/// process will happen more quickly. However, the estimated size may be
/// different from the originally specified size.
///
/// If the tracking state is PAUSED/STOPPED, this returns the estimated width
/// when the image state was last TRACKING. If the image state has never been
/// TRACKING, this returns 0, even the image has a specified physical size in
/// the image database.
void ArAugmentedImage_getExtentX(const ArSession *session,
                                 const ArAugmentedImage *augmented_image,
                                 float *out_extent_x);

/// Retrieves the estimated height, in metres, of the corresponding physical
/// image, as measured along the local Z-axis of the coordinate space with
/// origin and axes as defined by ArAugmentedImage_getCenterPose().
///
/// ARCore will attempt to estimate the physical image's height and continuously
/// update this estimate based on its understanding of the world. If an optional
/// physical size is specified in the image database, this estimation process
/// will happen more quickly. However, the estimated size may be different from
/// the originally specified size.
///
/// If the tracking state is PAUSED/STOPPED, this returns the estimated height
/// when the image state was last TRACKING. If the image state has never been
/// TRACKING, this returns 0, even the image has a specified physical size in
/// the image database.
void ArAugmentedImage_getExtentZ(const ArSession *session,
                                 const ArAugmentedImage *augmented_image,
                                 float *out_extent_z);

/// Returns the zero-based positional index of this image from its originating
/// image database.
///
/// This index serves as the unique identifier for the image in the database.
void ArAugmentedImage_getIndex(const ArSession *session,
                               const ArAugmentedImage *augmented_image,
                               int32_t *out_index);

/// Returns the name of this image.
///
/// The image name is not guaranteed to be unique.
///
/// This function will allocate memory for the name string, and set
/// *out_augmented_image_name to point to that string. The caller must release
/// the string using ArString_release when the string is no longer needed.
void ArAugmentedImage_acquireName(const ArSession *session,
                                  const ArAugmentedImage *augmented_image,
                                  char **out_augmented_image_name);

/// @}

// === ArAugmentedImageDatabase methods ===

/// @addtogroup augmented_image_database
/// @{

/// Creates a new empty image database.
void ArAugmentedImageDatabase_create(
    const ArSession *session,
    ArAugmentedImageDatabase **out_augmented_image_database);

/// Creates a new image database from a byte array. The contents of the byte
/// array must have been generated by the command-line database generation tool
/// provided in the SDK, or at runtime from ArAugmentedImageDatabase_serialize.
///
/// Note: this function takes about 10-20ms for a 5MB byte array. Run this in a
/// background thread if this affects your application.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_DATA_INVALID_FORMAT - the bytes are in an invalid format.
/// - #AR_ERROR_DATA_UNSUPPORTED_VERSION - the database is not supported by
///   this version of the SDK.
ArStatus ArAugmentedImageDatabase_deserialize(
    const ArSession *session,
    const uint8_t *database_raw_bytes,
    int64_t database_raw_bytes_size,
    ArAugmentedImageDatabase **out_augmented_image_database);

/// Serializes an image database to a byte array.
///
/// This function will allocate memory for the serialized raw byte array, and
/// set *out_image_database_raw_bytes to point to that byte array. The caller is
/// expected to release the byte array using ArByteArray_release when the byte
/// array is no longer needed.
void ArAugmentedImageDatabase_serialize(
    const ArSession *session,
    const ArAugmentedImageDatabase *augmented_image_database,
    uint8_t **out_image_database_raw_bytes,
    int64_t *out_image_database_raw_bytes_size);

/// Adds a single named image of unknown physical size to an image database,
/// from an array of grayscale pixel values. Returns the zero-based positional
/// index of the image within the image database.
///
/// If the physical size of the image is known, use
/// ArAugmentedImageDatabase_addImageWithPhysicalSize instead, to improve image
/// detection time.
///
/// For images added via ArAugmentedImageDatabase_addImage, ARCore estimates the
/// physical image's size and pose at runtime when the physical image is visible
/// and is being tracked. This extra estimation step will require the user to
/// move their device to view the physical image from different viewpoints
/// before the size and pose of the physical image can be estimated.
///
/// This function takes time to perform non-trivial image processing (20ms -
/// 30ms), and should be run on a background thread.
///
/// The image name is expected to be a null-terminated string in UTF-8 format.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_IMAGE_INSUFFICIENT_QUALITY - image quality is insufficient, e.g.
///   because of lack of features in the image.
ArStatus ArAugmentedImageDatabase_addImage(
    const ArSession *session,
    ArAugmentedImageDatabase *augmented_image_database,
    const char *image_name,
    const uint8_t *image_grayscale_pixels,
    int32_t image_width_in_pixels,
    int32_t image_height_in_pixels,
    int32_t image_stride_in_pixels,
    int32_t *out_index);

/// Adds a single named image to an image database, from an array of grayscale
/// pixel values, along with a positive physical width in meters for this image.
/// Returns the zero-based positional index of the image within the image
/// database.
///
/// If the physical size of the image is not known, use
/// ArAugmentedImageDatabase_addImage instead, at the expense of an increased
/// image detection time.
///
/// For images added via ArAugmentedImageDatabase_addImageWithPhysicalSize,
/// ARCore can estimate the pose of the physical image at runtime as soon as
/// ARCore detects the physical image, without requiring the user to move the
/// device to view the physical image from different viewpoints. Note that
/// ARCore will refine the estimated size and pose of the physical image as it
/// is viewed from different viewpoints.
///
/// This function takes time to perform non-trivial image processing (20ms -
/// 30ms), and should be run on a background thread.
///
/// The image name is expected to be a null-terminated string in UTF-8 format.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_IMAGE_INSUFFICIENT_QUALITY - image quality is insufficient, e.g.
///   because of lack of features in the image.
/// - #AR_ERROR_INVALID_ARGUMENT - image_width_in_meters is <= 0.
ArStatus ArAugmentedImageDatabase_addImageWithPhysicalSize(
    const ArSession *session,
    ArAugmentedImageDatabase *augmented_image_database,
    const char *image_name,
    const uint8_t *image_grayscale_pixels,
    int32_t image_width_in_pixels,
    int32_t image_height_in_pixels,
    int32_t image_stride_in_pixels,
    float image_width_in_meters,
    int32_t *out_index);

/// Returns the number of images in the image database.
void ArAugmentedImageDatabase_getNumImages(
    const ArSession *session,
    const ArAugmentedImageDatabase *augmented_image_database,
    int32_t *out_num_images);

/// Releases memory used by an image database.
void ArAugmentedImageDatabase_destroy(
    ArAugmentedImageDatabase *augmented_image_database);

/// @}

// === ArHitResultList methods ===

/// @addtogroup hit
/// @{

/// Creates a hit result list object.
void ArHitResultList_create(const ArSession *session,
                            ArHitResultList **out_hit_result_list);

/// Releases the memory used by a hit result list object, along with all the
/// trackable references it holds.
void ArHitResultList_destroy(ArHitResultList *hit_result_list);

/// Retrieves the number of hit results in this list.
void ArHitResultList_getSize(const ArSession *session,
                             const ArHitResultList *hit_result_list,
                             int32_t *out_size);

/// Copies an indexed entry in the list.  This acquires a reference to any
/// trackable referenced by the item, and releases any reference currently held
/// by the provided result object.
///
/// @param[in]    session           The ARCore session.
/// @param[in]    hit_result_list   The list from which to copy an item.
/// @param[in]    index             Index of the entry to copy.
/// @param[inout] out_hit_result    An already-allocated ArHitResult object into
///     which the result will be copied.
void ArHitResultList_getItem(const ArSession *session,
                             const ArHitResultList *hit_result_list,
                             int32_t index,
                             ArHitResult *out_hit_result);

// === ArHitResult methods ===

/// Allocates an empty hit result object.
void ArHitResult_create(const ArSession *session, ArHitResult **out_hit_result);

/// Releases the memory used by a hit result object, along with any
/// trackable reference it holds.
void ArHitResult_destroy(ArHitResult *hit_result);

/// Returns the distance from the camera to the hit location, in meters.
void ArHitResult_getDistance(const ArSession *session,
                             const ArHitResult *hit_result,
                             float *out_distance);

/// Returns the pose of the intersection between a ray and detected real-world
/// geometry. The position is the location in space where the ray intersected
/// the geometry. The orientation is a best effort to face the user's device,
/// and its exact definition differs depending on the Trackable that was hit.
///
/// ::ArPlane : X+ is perpendicular to the cast ray and parallel to the plane,
/// Y+ points along the plane normal (up, for #AR_PLANE_HORIZONTAL_UPWARD_FACING
/// planes), and Z+ is parallel to the plane, pointing roughly toward the
/// user's device.
///
/// ::ArPoint :
/// Attempt to estimate the normal of the surface centered around the hit test.
/// Surface normal estimation is most likely to succeed on textured surfaces
/// and with camera motion.
/// If ArPoint_getOrientationMode() returns ESTIMATED_SURFACE_NORMAL,
/// then X+ is perpendicular to the cast ray and parallel to the physical
/// surface centered around the hit test, Y+ points along the estimated surface
/// normal, and Z+ points roughly toward the user's device. If
/// ArPoint_getOrientationMode() returns INITIALIZED_TO_IDENTITY, then X+ is
/// perpendicular to the cast ray and points right from the perspective of the
/// user's device, Y+ points up, and Z+ points roughly toward the user's device.
///
/// If you wish to retain the location of this pose beyond the duration of a
/// single frame, create an anchor using ArHitResult_acquireNewAnchor() to save
/// the pose in a physically consistent way.
///
/// @param[in]    session    The ARCore session.
/// @param[in]    hit_result The hit result to retrieve the pose of.
/// @param[inout] out_pose   An already-allocated ArPose object into which the
///     pose will be stored.
void ArHitResult_getHitPose(const ArSession *session,
                            const ArHitResult *hit_result,
                            ArPose *out_pose);

/// Acquires reference to the hit trackable. This call must be paired with a
/// call to ArTrackable_release().
void ArHitResult_acquireTrackable(const ArSession *session,
                                  const ArHitResult *hit_result,
                                  ArTrackable **out_trackable);

/// Creates a new anchor at the hit location. See ArHitResult_getHitPose() for
/// details.  This is equivalent to creating an anchor on the hit trackable at
/// the hit pose.
///
/// @return #AR_SUCCESS or any of:
/// - #AR_ERROR_NOT_TRACKING
/// - #AR_ERROR_SESSION_PAUSED
/// - #AR_ERROR_RESOURCE_EXHAUSTED
/// - #AR_ERROR_DEADLINE_EXCEEDED - hit result must be used before the next call
///     to update().
ArStatus ArHitResult_acquireNewAnchor(ArSession *session,
                                      ArHitResult *hit_result,
                                      ArAnchor **out_anchor);

/// @}

// Utility methods for releasing data.

/// Releases a string acquired using an ARCore API function.
///
/// @param[in] str The string to be released.
void ArString_release(char *str);

/// Releases a byte array created using an ARCore API function.
void ArByteArray_release(uint8_t *byte_array);

#ifdef __cplusplus
}
#endif

#endif  // ARCORE_C_API_H_
