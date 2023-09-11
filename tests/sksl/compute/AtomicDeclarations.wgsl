diagnostic(off, derivative_uniformity);
struct ssbo {
  ssboAtomic: atomic<u32>,
  ssboAtomicArray: array<atomic<u32>, 2>,
  ssboStructWithAtomicMember: S,
  ssboStructWithAtomicMemberArray: array<_skArrayElement_S, 2>,
  ssboNestedStructWithAtomicMember: NestedS,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : ssbo;
struct S {
  structMemberAtomic: atomic<u32>,
  structMemberAtomicArray: array<atomic<u32>, 2>,
};
struct NestedS {
  nestedStructWithAtomicMember: S,
};
var<workgroup> wgAtomic: atomic<u32>;
var<workgroup> wgAtomicArray: array<atomic<u32>, 2>;
var<workgroup> wgNestedStructWithAtomicMember: NestedS;
fn _skslMain() {
  {
    let _skTemp1 = atomicLoad(&wgAtomic);
    let _skTemp2 = atomicAdd(&wgAtomicArray[1], _skTemp1);
    let _skTemp3 = atomicLoad(&wgAtomicArray[1]);
    let _skTemp4 = atomicAdd(&wgAtomicArray[0], _skTemp3);
    let _skTemp5 = atomicAdd(&wgNestedStructWithAtomicMember.nestedStructWithAtomicMember.structMemberAtomic, 1u);
  }
}
@compute @workgroup_size(64, 1, 1) fn main() {
  _skslMain();
}
struct _skArrayElement_S {
  @size(16) e : S
};
