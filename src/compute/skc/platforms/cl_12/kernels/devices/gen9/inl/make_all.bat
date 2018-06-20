@ECHO OFF

CMD /C make_inl_cl.bat  ..\..\..\block_pool_init.cl
CMD /C make_inl_cl.bat  ..\..\..\fills_expand.cl
CMD /C make_inl_cl.bat  ..\..\..\paths_copy.cl
CMD /C make_inl_cl.bat  ..\..\..\rasterize.cl
CMD /C make_inl_cl.bat  ..\..\..\segment_ttrk.cl
CMD /C make_inl_cl.bat  ..\..\..\rasters_alloc.cl
CMD /C make_inl_cl.bat  ..\..\..\prefix.cl
CMD /C make_inl_cl.bat  ..\..\..\place.cl
CMD /C make_inl_cl.bat  ..\..\..\segment_ttck.cl
CMD /C make_inl_cl.bat  ..\..\..\render.cl
CMD /C make_inl_cl.bat  ..\..\..\paths_reclaim.cl
CMD /C make_inl_cl.bat  ..\..\..\rasters_reclaim.cl

