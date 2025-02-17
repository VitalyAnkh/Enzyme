// RUN: %eopt --enzyme-batch %s | FileCheck %s

module {
  func.func @square(%x : f64) -> f64 {
    %y = math.sin %x : f64
    return %y : f64
  }
  func.func @dsq(%x : tensor<10x2xf64>) -> tensor<10x2xf64> {
    %r = enzyme.batch @square(%x) { batch_shape=array<i64: 10, 2> } : (tensor<10x2xf64>) -> (tensor<10x2xf64>)
    return %r : tensor<10x2xf64>
  }
}

// CHECK:  func.func @dsq(%arg0: tensor<10x2xf64>) -> tensor<10x2xf64> {
// CHECK-NEXT:    %0 = call @batched_square(%arg0) : (tensor<10x2xf64>) -> tensor<10x2xf64>
// CHECK-NEXT:    return %0 : tensor<10x2xf64>
// CHECK-NEXT:  }
// CHECK:  func.func private @batched_square(%arg0: tensor<10x2xf64>) -> tensor<10x2xf64> {
// CHECK-NEXT:    %0 = math.sin %arg0 : tensor<10x2xf64>
// CHECK-NEXT:    return %0 : tensor<10x2xf64>
// CHECK-NEXT:  }