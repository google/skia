SkSurface Reference
===

# <a name='Surface'>Surface</a>

# <a name='SkSurface'>Class SkSurface</a>
<a href='#SkSurface'>SkSurface</a> is responsible for managing the pixels that a canvas draws into. The pixels can be
allocated either in CPU memory (a raster surface) or on the GPU (a <a href='undocumented#GrRenderTarget'>GrRenderTarget</a> surface).
<a href='#SkSurface'>SkSurface</a> takes care of allocating a <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that will draw into the surface. Call
surface-><a href='#SkSurface_getCanvas'>getCanvas</a> to use that canvas (but don't delete it, it is owned by the surface).
<a href='#SkSurface'>SkSurface</a> always has non-zero dimensions. If there is a request for a new surface, and either
of the requested dimensions are zero, then nullptr will be returned.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constant'>Constants</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>enum and enum class, and their const values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkSurface'>SkSurface</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Constant'>Constant</a>


SkSurface related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>options to read and write back-end object</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>parameter options for <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite BackendHandleAccess</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>back-end object must be overwritten</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard ContentChangeMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>discards surface on change</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead BackendHandleAccess</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>back-end object is readable</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite BackendHandleAccess</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>back-end object is writable</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_kRetain_ContentChangeMode'>kRetain ContentChangeMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>preserves surface on change</td>
  </tr>
</table>

## <a name='Related_Function'>Related Function</a>


SkSurface global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Miscellaneous'>Miscellaneous</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>other functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Pixels'>Pixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions with pixel access</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Property'>Property</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Utility'>Utility</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>rarely called management functions</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkSurface member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU render target</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU texture</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU back-end render target</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeNull'>MakeNull</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> without backing pixels</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRaster'>MakeRaster</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Storage'>Pixel Storage</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Storage'>Pixel Storage</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from width, height matching output</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> pointing to new GPU memory buffer</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_characterize'>characterize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Surface_Characterization'>Surface Characterization</a> for threaded GPU processing</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_draw'>draw</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>draws <a href='#Surface'>Surface</a> contents to canvas</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_flush'>flush</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>resolves pending I/O</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>resolves pending I/O, and signal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_generationID'>generationID</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns unique ID</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the GPU reference to render target</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getBackendTexture'>getBackendTexture</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the GPU reference to texture</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getCanvas'>getCanvas</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkCanvas_Reference#Canvas'>Canvas</a> that draws into <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_height'>height</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns pixel row count</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='SkImage_Reference#Image'>Image</a> capturing <a href='#Surface'>Surface</a> contents</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_makeSurface'>makeSurface</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates a compatible <a href='#Surface'>Surface</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>notifies that contents will be changed outside of Skia</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_peekPixels'>peekPixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='#Surface'>Surface</a> parameters to <a href='SkPixmap_Reference#Pixmap'>Pixmap</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_props'>props</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Surface_Properties'>Surface Properties</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_readPixels'>readPixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_wait'>wait</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>pauses commands until signaled</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_width'>width</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns pixel column count</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_writePixels'>writePixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels</td>
  </tr>
</table>

## <a name='Constructor'>Constructor</a>


SkSurface can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU render target</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU texture</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from GPU back-end render target</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeNull'>MakeNull</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> without backing pixels</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRaster'>MakeRaster</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Storage'>Pixel Storage</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Storage'>Pixel Storage</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> from width, height matching output</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='#Surface'>Surface</a> pointing to new GPU memory buffer</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates <a href='SkImage_Reference#Image'>Image</a> capturing <a href='#Surface'>Surface</a> contents</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_makeSurface'>makeSurface</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>creates a compatible <a href='#Surface'>Surface</a></td>
  </tr>
</table>

<a name='SkSurface_MakeRasterDirect'></a>
## MakeRasterDirect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels, size_t rowBytes,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='#Surface'>Surface</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Surface'>Surface</a> draws directly into <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a>.

<a href='#Surface'>Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> supported by <a href='undocumented#Raster_Surface'>Raster Surface</a>;
<a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> is not nullptr;
<a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a> is large enough to contain info width <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> of <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer size should be info height times computed <a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a>.
<a href='#Pixels'>Pixels</a> are not initialized.
To access <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> after drawing, call <a href='#SkSurface_flush'>flush</a> or <a href='#SkSurface_peekPixels'>peekPixels</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width and height must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="3f5aeb870104187643197354a7f1d27a">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='SkCanvas_Reference#SkCanvas_MakeRasterDirect'>SkCanvas::MakeRasterDirect</a>

---

<a name='SkSurface_MakeRasterDirectReleaseProc'></a>
## MakeRasterDirectReleaseProc

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                           size_t rowBytes, void (*releaseProc) (void* pixels,
                                           void* context) , void* context,
                                           const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='#Surface'>Surface</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Surface'>Surface</a> draws directly into <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a>.
<a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a> is called with <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> and <a href='#SkSurface_MakeRasterDirectReleaseProc_context'>context</a> when <a href='#Surface'>Surface</a> is deleted.

<a href='#Surface'>Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> supported by <a href='undocumented#Raster_Surface'>Raster Surface</a>;
<a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> is not nullptr;
<a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a> is large enough to contain info width <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> of <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer size should be info height times computed <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a>.
<a href='#Pixels'>Pixels</a> are not initialized.
To access <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> after drawing, call <a href='#SkSurface_flush'>flush</a> or <a href='#SkSurface_peekPixels'>peekPixels</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width and height must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>called when <a href='#Surface'>Surface</a> is deleted; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_context'><code><strong>context</strong></code></a></td>
    <td>passed to <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="8e6530b26ab4096a9a91cfaadda1c568">

#### Example Output

~~~~
---
-x-
---
expected release context
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup>

---

<a name='SkSurface_MakeRaster'></a>
## MakeRaster

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t rowBytes,
                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Allocates raster <a href='#Surface'>Surface</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Surface'>Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href='undocumented#Pixel'>Pixel</a> memory size is <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.<a href='#SkSurface_height'>height</a> times
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>, or times <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.minRowBytes() if <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is zero.
<a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='#Surface'>Surface</a> is deleted.

<a href='#Surface'>Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> supported by <a href='undocumented#Raster_Surface'>Raster Surface</a>;
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is large enough to contain info width pixels of <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, or is zero.

If <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is not zero, subsequent images returned by <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>
have the same <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width and height must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next; may be zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="a803910ada4f8733f0b62456afead55f">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeRaster_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr)
</pre>

Allocates raster <a href='#Surface'>Surface</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Surface'>Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href='undocumented#Pixel'>Pixel</a> memory size is <a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.<a href='#SkSurface_height'>height</a> times
<a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.minRowBytes().
<a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='#Surface'>Surface</a> is deleted.

<a href='#Surface'>Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> supported by <a href='undocumented#Raster_Surface'>Raster Surface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width and height must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="c6197d204ef9e4ccfb583242651fb2a7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeRasterN32Premul'></a>
## MakeRasterN32Premul

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(int width, int height,
                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='#Surface'>Surface</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Surface'>Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href='undocumented#Pixel'>Pixel</a> memory size is height times width times
four. <a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='#Surface'>Surface</a> is deleted.

Internally, sets <a href='SkImageInfo_Reference#Image_Info'>Image Info</a> to width, height, <a href='SkImageInfo_Reference#kN32_SkColorType'>Native_Color_Type</a>, and
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

<a href='#Surface'>Surface</a> is returned if width and height are greater than zero.

Use to create <a href='#Surface'>Surface</a> that matches <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, the native pixel arrangement on
the platform. <a href='#Surface'>Surface</a> drawn to output device skips converting its pixel format.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_width'><code><strong>width</strong></code></a></td>
    <td>pixel column count; must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_height'><code><strong>height</strong></code></a></td>
    <td>pixel row count; must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="b932a2bd68455fb0af2e7a1ed19e36b3">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeFromBackendTexture'></a>
## MakeFromBackendTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                               const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                               <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                               <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                               const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed texture into <a href='#Surface'>Surface</a>. Caller must ensure the texture is
valid for the lifetime of returned <a href='#Surface'>Surface</a>. If <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>sampleCnt</a> greater than zero,
creates an intermediate MSAA <a href='#Surface'>Surface</a> which is used for drawing <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a>.

<a href='#Surface'>Surface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> is valid if
its pixel configuration agrees with <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendTexture_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> capabilities, and the <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> must be able to support
back-end textures.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="d3aec071998f871809f515e58abb1b0e" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeFromBackendRenderTarget'></a>
## MakeFromBackendRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   const <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& backendRenderTarget,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                   <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed buffer into <a href='#Surface'>Surface</a>. Caller must ensure <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a>
is valid for the lifetime of returned <a href='#Surface'>Surface</a>.

<a href='#Surface'>Surface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> is valid if
its pixel configuration agrees with <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> capabilities, and the <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> must be able to support
back-end render targets.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'><code><strong>backendRenderTarget</strong></code></a></td>
    <td>GPU intermediate memory buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<pre style="padding: 1em 1em 1em 1em; font-size: 13px width: 62.5em; background-color: #f0f0f0">

    SkPaint paint;
    paint.setTextSize(32);
    GrContext* context = canvas->getGrContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, paint);
         return;
    }
    sk_sp<SkSurface> gpuSurface = SkSurface::MakeFromBackendRenderTarget(context,
            backEndRenderTarget, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
            nullptr, nullptr);
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->drawString("GPU rocks!", 20, 40, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);

</pre>

### See Also

<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeFromBackendTextureAsRenderTarget'></a>
## MakeFromBackendTextureAsRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType, <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed texture into <a href='#Surface'>Surface</a>. Caller must ensure <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> is
valid for the lifetime of returned <a href='#Surface'>Surface</a>. If <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>sampleCnt</a> greater than zero,
creates an intermediate MSAA <a href='#Surface'>Surface</a> which is used for drawing <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a>.

<a href='#Surface'>Surface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> is valid if
its pixel configuration agrees with <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> capabilities.

Returned <a href='#Surface'>Surface</a> is available only for drawing into, and cannot generate an
<a href='SkImage_Reference#Image'>Image</a>.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="5e87093b9cbe95124ae14cbe77091eb7" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeRenderTarget'></a>
## MakeRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                         <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps,
                                         bool shouldCreateWithMips = false)
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> in ImageInfo.  <a href='#SkSurface_MakeRenderTarget_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_imageInfo'>imageInfo</a>
describes the pixel format in <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, and transparency in
<a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, and color matching in <a href='undocumented#Color_Space'>Color Space</a>.

<a href='#SkSurface_MakeRenderTarget_sampleCount'>sampleCount</a> requests the number of samples per pixel.
Pass zero to disable <a href='undocumented#Multi_Sample_Anti_Aliasing'>Multi Sample Anti Aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>surfaceOrigin</a> pins either the top-left or the bottom-left corner to the origin.

<a href='#SkSurface_MakeRenderTarget_shouldCreateWithMips'>shouldCreateWithMips</a> hints that <a href='SkImage_Reference#Image'>Image</a> returned by <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> is <a href='undocumented#Mip_Map'>Mip Map</a>.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted_kNo'>SkBudgeted::kNo</a>, <a href='undocumented#SkBudgeted_kYes'>SkBudgeted::kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>;
width, or height, or both, may be zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_shouldCreateWithMips'><code><strong>shouldCreateWithMips</strong></code></a></td>
    <td>hint that <a href='#Surface'>Surface</a> will host <a href='undocumented#Mip_Map'>Mip Map</a> images</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="67b6609471a3f1ed0f4b1657004cdecb" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props)
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> in ImageInfo.  <a href='#SkSurface_MakeRenderTarget_2_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_2_imageInfo'>imageInfo</a>
describes the pixel format in <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, and transparency in
<a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, and color matching in <a href='undocumented#Color_Space'>Color Space</a>.

<a href='#SkSurface_MakeRenderTarget_2_sampleCount'>sampleCount</a> requests the number of samples per pixel.
Pass zero to disable <a href='undocumented#Multi_Sample_Anti_Aliasing'>Multi Sample Anti Aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='#Surface'>Surface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted_kNo'>SkBudgeted::kNo</a>, <a href='undocumented#SkBudgeted_kYes'>SkBudgeted::kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width, or height, or both, may be zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per pixel, or 0 to disable <a href='undocumented#Multi_Sample_Anti_Aliasing'>Multi Sample Anti Aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="640321e8ecfb3f9329f3bc6e1f02485f" gpu="true" cpu="true"><div>LCD text takes advantage of raster striping to improve resolution. Only one of
the four combinations is correct, depending on whether monitor LCD striping is
horizontal or vertical, and whether the order of the stripes is red blue green
or red green blue.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo)
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> in ImageInfo.  <a href='#SkSurface_MakeRenderTarget_3_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_3_imageInfo'>imageInfo</a>
describes the pixel format in <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, and transparency in
<a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, and color matching in <a href='undocumented#Color_Space'>Color Space</a>.

<a href='#Surface'>Surface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted_kNo'>SkBudgeted::kNo</a>, <a href='undocumented#SkBudgeted_kYes'>SkBudgeted::kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='undocumented#Raster_Surface'>Raster Surface</a>; width, or height, or both, may be zero</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="5c7629c15e9ac93f098335e72560fa2e" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                         const <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& characterization,
                                         <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted)
</pre>

Returns <a href='#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_4_context'>context</a> that is compatible with the provided
<a href='#SkSurface_MakeRenderTarget_4_characterization'>characterization</a>. <a href='#SkSurface_MakeRenderTarget_4_budgeted'>budgeted</a> selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_4_context'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_characterization'><code><strong>characterization</strong></code></a></td>
    <td>description of the desired <a href='#SkSurface'>SkSurface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted_kNo'>SkBudgeted::kNo</a>, <a href='undocumented#SkBudgeted_kYes'>SkBudgeted::kYes</a></td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid; otherwise, nullptr

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeNull'></a>
## MakeNull

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeNull'>MakeNull</a>(int width, int height)
</pre>

Returns <a href='#Surface'>Surface</a> without backing pixels. Drawing to <a href='SkCanvas_Reference#Canvas'>Canvas</a> returned from <a href='#Surface'>Surface</a>
has no effect. Calling <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> on returned <a href='#Surface'>Surface</a> returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeNull_width'><code><strong>width</strong></code></a></td>
    <td>one or greater</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeNull_height'><code><strong>height</strong></code></a></td>
    <td>one or greater</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if width and height are positive; otherwise, nullptr

### Example

<div><fiddle-embed name="99a54b814ccab7d2b1143c88581649ff">

#### Example Output

~~~~
SkSurface::MakeNull(0, 0) == nullptr
surf->makeImageSnapshot() == nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

## <a name='Property'>Property</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_generationID'>generationID</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns unique ID</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the GPU reference to render target</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getBackendTexture'>getBackendTexture</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the GPU reference to texture</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_getCanvas'>getCanvas</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkCanvas_Reference#Canvas'>Canvas</a> that draws into <a href='#Surface'>Surface</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_height'>height</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns pixel row count</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_props'>props</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Surface_Properties'>Surface Properties</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_width'>width</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns pixel column count</td>
  </tr>
</table>

<a name='SkSurface_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_width'>width</a>() const
</pre>

Returns pixel count in each row; may be zero or greater.

### Return Value

number of pixel columns

### Example

<div><fiddle-embed name="df066b56dd97c7c589fd2bb6a2539de8">

#### Example Output

~~~~
surface width=37  canvas width=37
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_height'>height</a>

---

<a name='SkSurface_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_height'>height</a>() const
</pre>

Returns pixel row count; may be zero or greater.

### Return Value

number of pixel rows

### Example

<div><fiddle-embed name="20571cc23e3146deaa09046b64cc0aef">

#### Example Output

~~~~
surface height=1000  canvas height=1000
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_width'>width</a>

---

<a name='SkSurface_generationID'></a>
## generationID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkSurface_generationID'>generationID</a>()
</pre>

Returns unique value identifying the content of <a href='#Surface'>Surface</a>. Returned value changes
each time the content changes. Content is changed by drawing, or by calling
<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>.

### Return Value

unique content identifier

### Example

<div><fiddle-embed name="d8be8b6e59de244e4cbf58ec9554557b">

#### Example Output

~~~~
surface generationID: 1
surface generationID: 2
surface generationID: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

## <a name='SkSurface_ContentChangeMode'>Enum SkSurface::ContentChangeMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> {
        <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard ContentChangeMode</a>,
        <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain ContentChangeMode</a>,
    };
</pre>

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> members are parameters to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscard_ContentChangeMode'><code>SkSurface::kDiscard_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # discards surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> to discard surface contents when
the surface is cleared or overwritten.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kRetain_ContentChangeMode'><code>SkSurface::kRetain_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # preserves surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> when to preserve surface contents.
If a snapshot has been generated, this copies the <a href='#Surface'>Surface</a> contents.
</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_generationID'>generationID</a>

## <a name='Miscellaneous'>Miscellaneous</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>notifies that contents will be changed outside of Skia</td>
  </tr>
</table>

<a name='SkSurface_notifyContentWillChange'></a>
## notifyContentWillChange

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> mode)
</pre>

Notifies that <a href='#Surface'>Surface</a> contents will be changed by code outside of Skia.
Subsequent calls to <a href='#SkSurface_generationID'>generationID</a> return a different value.

<a href='#SkSurface_notifyContentWillChange_mode'>mode</a> is normally passed as <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain ContentChangeMode</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_notifyContentWillChange_mode'><code><strong>mode</strong></code></a></td>
    <td>one of: <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard ContentChangeMode</a>, <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain ContentChangeMode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="be9574c4a14f891e1abb4ec2b1e51d6c"></fiddle-embed></div>

### See Also

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_generationID'>generationID</a>

---

## <a name='SkSurface_BackendHandleAccess'>Enum SkSurface::BackendHandleAccess</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> {
        <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead BackendHandleAccess</a>,
        <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite BackendHandleAccess</a>,
        <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite BackendHandleAccess</a>,
    };

    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushRead_TextureHandleAccess'>kFlushRead TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushWrite_TextureHandleAccess'>kFlushWrite TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kDiscardWrite_TextureHandleAccess'>kDiscardWrite TextureHandleAccess</a> =
            <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite BackendHandleAccess</a>;
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushRead_BackendHandleAccess'><code>SkSurface::kFlushRead_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller may read from the back-end object.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushWrite_BackendHandleAccess'><code>SkSurface::kFlushWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller may write to the back-end object.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscardWrite_BackendHandleAccess'><code>SkSurface::kDiscardWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller must overwrite the entire back-end object.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushRead_TextureHandleAccess'><code>SkSurface::kFlushRead_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushWrite_TextureHandleAccess'><code>SkSurface::kFlushWrite_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscardWrite_TextureHandleAccess'><code>SkSurface::kDiscardWrite_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_getBackendTexture'>getBackendTexture</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

<a name='SkSurface_getBackendTexture'></a>
## getBackendTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess)
</pre>

Retrieves the back-end texture. If <a href='#Surface'>Surface</a> has no back-end texture, an invalid
object is returned. Call <a href='undocumented#GrBackendTexture_isValid'>GrBackendTexture::isValid</a> to determine if the result
is valid.

The returned <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> should be discarded if the <a href='#Surface'>Surface</a> is drawn to or deleted.

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendTexture_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of:  <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead BackendHandleAccess</a>,
<a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite BackendHandleAccess</a>, <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite BackendHandleAccess</a></td>
  </tr>
</table>

### Return Value

GPU texture reference; invalid on failure

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

---

<a name='SkSurface_getBackendRenderTarget'></a>
## getBackendRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess)
</pre>

Retrieves the back-end render target. If <a href='#Surface'>Surface</a> has no back-end render target, an invalid
object is returned. Call <a href='undocumented#GrBackendRenderTarget_isValid'>GrBackendRenderTarget::isValid</a> to determine if the result
is valid.

The returned <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> should be discarded if the <a href='#Surface'>Surface</a> is drawn to
or deleted.

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendRenderTarget_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of:  <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead BackendHandleAccess</a>,
<a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite BackendHandleAccess</a>, <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite BackendHandleAccess</a></td>
  </tr>
</table>

### Return Value

GPU render target reference; invalid on failure

### See Also

<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>

---

<a name='SkSurface_getCanvas'></a>
## getCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkSurface_getCanvas'>getCanvas</a>()
</pre>

Returns <a href='SkCanvas_Reference#Canvas'>Canvas</a> that draws into <a href='#Surface'>Surface</a>. Subsequent calls return the same <a href='SkCanvas_Reference#Canvas'>Canvas</a>.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> returned is managed and owned by <a href='#Surface'>Surface</a>, and is deleted when <a href='#Surface'>Surface</a>
is deleted.

### Return Value

drawing <a href='SkCanvas_Reference#Canvas'>Canvas</a> for <a href='#Surface'>Surface</a>

### Example

<div><fiddle-embed name="33d0c5ad5a4810e533ae1010e29f8b75"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeSurface'>makeSurface</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup>

---

<a name='SkSurface_makeSurface'></a>
## makeSurface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo)
</pre>

Returns a compatible <a href='#Surface'>Surface</a>, or nullptr. Returned <a href='#Surface'>Surface</a> contains
the same raster, GPU, or null properties as the original. Returned <a href='#Surface'>Surface</a>
does not share the same pixels.

Returns nullptr if <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a> width or height are zero, or if <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a>
is incompatible with <a href='#Surface'>Surface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_makeSurface_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>, <a href='undocumented#Color_Space'>Color Space</a>,
of <a href='#Surface'>Surface</a>; width and height must be greater than zero</td>
  </tr>
</table>

### Return Value

compatible <a href='#Surface'>Surface</a> or nullptr

### Example

<div><fiddle-embed name="8f91f58001d9c10420eb146fbc169af4"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a> <a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup>

---

<a name='SkSurface_makeImageSnapshot'></a>
## makeImageSnapshot

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>()
</pre>

Returns <a href='SkImage_Reference#Image'>Image</a> capturing <a href='#Surface'>Surface</a> contents. Subsequent drawing to <a href='#Surface'>Surface</a> contents
are not captured. <a href='SkImage_Reference#Image'>Image</a> allocation is accounted for if <a href='#Surface'>Surface</a> was created with
<a href='undocumented#SkBudgeted_kYes'>SkBudgeted::kYes</a>.

### Return Value

<a href='SkImage_Reference#Image'>Image</a> initialized with <a href='#Surface'>Surface</a> contents

### Example

<div><fiddle-embed name="46f1fa0d95e590a64bed0140407ce5f7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

## <a name='Pixels'>Pixels</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_draw'>draw</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>draws <a href='#Surface'>Surface</a> contents to canvas</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_draw'>draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_draw_2'>draw(SkDeferredDisplayList* deferredDisplayList)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_peekPixels'>peekPixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='#Surface'>Surface</a> parameters to <a href='SkPixmap_Reference#Pixmap'>Pixmap</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_readPixels'>readPixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_readPixels'>readPixels(const SkPixmap& dst, int srcX, int srcY)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_readPixels_2'>readPixels(const SkImageInfo& dstInfo, void* dstPixels, size t dstRowBytes, int srcX, int srcY)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_readPixels_3'>readPixels(const SkBitmap& dst, int srcX, int srcY)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_writePixels'>writePixels</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_writePixels'>writePixels(const SkPixmap& src, int dstX, int dstY)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_writePixels_2'>writePixels(const SkBitmap& src, int dstX, int dstY)</a></td>
  </tr>
</table>

<a name='SkSurface_draw'></a>
## draw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* canvas, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint)
</pre>

Draws <a href='#Surface'>Surface</a> contents to <a href='#SkSurface_draw_canvas'>canvas</a>, with its top-left corner at (<a href='#SkSurface_draw_x'>x</a>, <a href='#SkSurface_draw_y'>y</a>).

If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkSurface_draw_paint'>paint</a> is not nullptr, apply <a href='undocumented#Color_Filter'>Color Filter</a>, <a href='SkColor_Reference#Alpha'>Color Alpha</a>, <a href='undocumented#Image_Filter'>Image Filter</a>,
<a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>, and <a href='undocumented#Draw_Looper'>Draw Looper</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> drawn into</td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset in <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset in <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>, <a href='undocumented#Color_Filter'>Color Filter</a>, <a href='undocumented#Image_Filter'>Image Filter</a>,
and so on; or nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0de693f4d8dd898a60be8cfba23952be"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

<a name='SkSurface_peekPixels'></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* pixmap)
</pre>

Copies <a href='#Surface'>Surface</a> pixel address, row bytes, and <a href='SkImageInfo_Reference#Image_Info'>Image Info</a> to <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, if address
is available, and returns true. If pixel address is not available, return
false and leave <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> unchanged.

<a href='#SkSurface_peekPixels_pixmap'>pixmap</a> contents become invalid on any future change to <a href='#Surface'>Surface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixel state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href='#Surface'>Surface</a> has direct access to pixels

### Example

<div><fiddle-embed name="8c6184f22cfe068f021704cf92a147a1"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels'></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkSurface_readPixels_dst'>dst</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_srcX'>srcX</a>, <a href='#SkSurface_readPixels_srcY'>srcY</a>) and <a href='#Surface'>Surface</a> (<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_height'>height</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkSurface_height'>height</a>).
Copies each readable pixel intersecting both rectangles, without scaling,
converting to <a href='#SkSurface_readPixels_dst'>dst</a>.colorType() and <a href='#SkSurface_readPixels_dst'>dst</a>.alphaType() if required.

<a href='#Pixels'>Pixels</a> are readable when <a href='#Surface'>Surface</a> is raster, or backed by a GPU.

The destination pixel storage must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_dst'>dst</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_srcX'>srcX</a> or <a href='#SkSurface_readPixels_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_dst'>dst</a>.rowBytes() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="9f454fb93bca6482598d198b4121f0a6"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_2_srcX'>srcX</a>, <a href='#SkSurface_readPixels_2_srcY'>srcY</a>) and <a href='#Surface'>Surface</a> (<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_height'>height</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkSurface_height'>height</a>).
Copies each readable pixel intersecting both rectangles, without scaling,
converting to <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.colorType() and <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.alphaType() if required.

<a href='#Pixels'>Pixels</a> are readable when <a href='#Surface'>Surface</a> is raster, or backed by a GPU.

The destination pixel storage must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_2_srcX'>srcX</a> or <a href='#SkSurface_readPixels_2_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.colorType() or <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.alphaType().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a> is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#Color_Type'>Color Type</a>, and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> of <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkSurface_height'>height</a> times <a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>size of one destination row; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkSurface_width'>width</a> times pixel size, or larger</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="484d60dab5d846bf28c7a4d48892324a"><div>A black oval drawn on a red background provides an image to copy.
<a href='#SkSurface_readPixels'>readPixels</a> copies one quarter of the <a href='#Surface'>Surface</a> into each of the four corners.
The copied quarter ovals overdraw the original oval.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& dst, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Surface'>Surface</a> into bitmap.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_3_srcX'>srcX</a>, <a href='#SkSurface_readPixels_3_srcY'>srcY</a>) and <a href='#Surface'>Surface</a> (<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_height'>height</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (bitmap.<a href='#SkSurface_width'>width</a>, bitmap.<a href='#SkSurface_height'>height</a>).
Copies each readable pixel intersecting both rectangles, without scaling,
converting to bitmap.colorType() and bitmap.alphaType() if required.

<a href='#Pixels'>Pixels</a> are readable when <a href='#Surface'>Surface</a> is raster, or backed by a GPU.

The destination pixel storage must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> and <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_3_dst'>dst</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_3_srcX'>srcX</a> or <a href='#SkSurface_readPixels_3_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_3_dst'>dst</a>.colorType() or <a href='#SkSurface_readPixels_3_dst'>dst</a>.alphaType().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a> pixels could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a>.rowBytes() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="2d991a231e49d1de13eeb2ba9b440e01"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_writePixels'></a>
## writePixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_src'>src</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> to the <a href='#Surface'>Surface</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkSurface_height'>height</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_writePixels_dstX'>dstX</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a>) and(<a href='#SkSurface_writePixels_dstX'>dstX</a> + <a href='#Surface'>Surface</a> <a href='#SkSurface_width'>width</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a> + <a href='#Surface'>Surface</a> <a href='#SkSurface_height'>height</a>)
.

Copies each readable pixel intersecting both rectangles, without scaling,
converting to <a href='#Surface'>Surface</a> colorType() and <a href='#Surface'>Surface</a> alphaType() if required.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="760793bcf0ef193fa61ea03e6e8fc825"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_peekPixels'>peekPixels</a>

---

<a name='SkSurface_writePixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, int dstX, int dstY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_2_src'>src</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to the <a href='#Surface'>Surface</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkSurface_width'>width</a>, <a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkSurface_height'>height</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_writePixels_2_dstX'>dstX</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a>) and(<a href='#SkSurface_writePixels_2_dstX'>dstX</a> + <a href='#Surface'>Surface</a> <a href='#SkSurface_width'>width</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a> + <a href='#Surface'>Surface</a> <a href='#SkSurface_height'>height</a>)
.

Copies each readable pixel intersecting both rectangles, without scaling,
converting to <a href='#Surface'>Surface</a> colorType() and <a href='#Surface'>Surface</a> alphaType() if required.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="af0b72360c1c7a25b4754bfa47011dd5"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_peekPixels'>peekPixels</a>

---

<a name='SkSurface_props'></a>
## props

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props</a>() const
</pre>

Returns <a href='undocumented#Surface_Properties'>Surface Properties</a> for surface.

### Return Value

LCD striping orientation and setting for device independent fonts

### Example

<div><fiddle-embed name="13cf9e7b2894ae6e98c1fd719040bf01">

#### Example Output

~~~~
surf.props(): kRGB_H_SkPixelGeometry
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

---

<a name='SkSurface_prepareForExternalIO'></a>
## prepareForExternalIO

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a>()
</pre>

Deprecated.

soon

---

## <a name='Utility'>Utility</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_characterize'>characterize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Surface_Characterization'>Surface Characterization</a> for threaded GPU processing</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_flush'>flush</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>resolves pending I/O</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>resolves pending I/O, and signal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkSurface_wait'>wait</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>pauses commands until signaled</td>
  </tr>
</table>

<a name='SkSurface_flush'></a>
## flush

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_flush'>flush</a>()
</pre>

Issues pending <a href='#Surface'>Surface</a> commands to the GPU-backed API and resolves any <a href='#Surface'>Surface</a> MSAA.

Skia flushes as needed, so it is not necessary to call this if Skia manages
drawing and object lifetime. Call when interleaving Skia calls with native
GPU calls.

### See Also

<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_flushAndSignalSemaphores'></a>
## flushAndSignalSemaphores

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(int numSemaphores,
                                               <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> signalSemaphores[])
</pre>

Issues pending <a href='#Surface'>Surface</a> commands to the GPU-backed API and resolves any <a href='#Surface'>Surface</a> MSAA.
After issuing all commands, <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> of count <a href='#SkSurface_flushAndSignalSemaphores_numSemaphores'>numSemaphores</a> semaphores
are signaled by the GPU.

For each <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> in <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>:
if <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> is initialized, the GPU back-end uses the semaphore as is;
otherwise, a new semaphore is created and initializes <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>.

The caller must delete the semaphores created and returned in <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>.
<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> can be deleted as soon as this function returns.

If the back-end API is OpenGL only uninitialized <a href='undocumented#Backend_Semaphore'>Backend Semaphores</a> are supported.

If the back-end API is Vulkan semaphores may be initialized or uninitialized.
If uninitialized, created semaphores are valid only with the VkDevice
with which they were created.

If <a href='undocumented#GrSemaphoresSubmitted_kNo'>GrSemaphoresSubmitted::kNo</a> is returned, the GPU back-end did not create or
add any semaphores to signal on the GPU; the caller should not instruct the GPU
to wait on any of the semaphores.

Pending surface commands are flushed regardless of the return result.

### Parameters

<table>  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td>size of <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_signalSemaphores'><code><strong>signalSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

one of: <a href='undocumented#GrSemaphoresSubmitted_kYes'>GrSemaphoresSubmitted::kYes</a>, <a href='undocumented#GrSemaphoresSubmitted_kNo'>GrSemaphoresSubmitted::kNo</a>

### See Also

<a href='#SkSurface_wait'>wait</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_wait'></a>
## wait

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_wait'>wait</a>(int numSemaphores, const <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* waitSemaphores)
</pre>

Inserts a list of GPU semaphores that the current GPU-backed API must wait on before
executing any more commands on the GPU for this surface. Skia will take ownership of the
underlying semaphores and delete them once they have been signaled and waited on.
If this call returns false, then the GPU back-end will not wait on any passed in semaphores,
and the client will still own the semaphores.

### Parameters

<table>  <tr>    <td><a name='SkSurface_wait_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td>size of <a href='#SkSurface_wait_waitSemaphores'>waitSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_wait_waitSemaphores'><code><strong>waitSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

true if GPU is waiting on semaphores

### See Also

<a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_characterize'></a>
## characterize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* characterization) const
</pre>

Initializes <a href='undocumented#Surface_Characterization'>Surface Characterization</a> that can be used to perform GPU back-end
processing in a separate thread. Typically this is used to divide drawing
into multiple tiles. DeferredDisplayListRecorder records the drawing commands
for each tile.

Return true if <a href='#Surface'>Surface</a> supports <a href='#SkSurface_characterize_characterization'>characterization</a>. <a href='undocumented#Raster_Surface'>Raster Surface</a> returns false.

### Parameters

<table>  <tr>    <td><a name='SkSurface_characterize_characterization'><code><strong>characterization</strong></code></a></td>
    <td>properties for parallel drawing</td>
  </tr>
</table>

### Return Value

true if supported

### Example

<div><fiddle-embed name="6de6f3ef699a72ff26da1b26b23a3316" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>

---

<a name='SkSurface_draw_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_draw'>draw</a>(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* deferredDisplayList)
</pre>

Draws deferred display list created using <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a>.
Has no effect and returns false if <a href='undocumented#Surface_Characterization'>Surface Characterization</a> stored in
<a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> is not compatible with <a href='#Surface'>Surface</a>.

<a href='undocumented#Raster_Surface'>Raster Surface</a> returns false.

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_2_deferredDisplayList'><code><strong>deferredDisplayList</strong></code></a></td>
    <td>drawing commands</td>
  </tr>
</table>

### Return Value

false if <a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> is not compatible

### Example

<div><fiddle-embed name="46d9bacf593deaaeabd74ff42f2571a0" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_characterize'>characterize</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>

---

