Ganesh files have been moved out of include/gpu/gl into include/gpu/ganesh/gl. Same
for d3d, mock, and vk. Shims have been left in place, but clients should migrate to
the new paths.