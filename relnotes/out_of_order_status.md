* Graphite's InsertStatus now has an additional kOutOfOrderRecording to differentiate this
  unrecoverable error from programming errors that would lead to kInvalidRecording. Out of order
  recordings can currently arise "naturally" if prior dependent recordings failed due to resource
  creation or update errors from the GPU driver.
