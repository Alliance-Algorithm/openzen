//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 4.0.2
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class ZenEventData : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal ZenEventData(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(ZenEventData obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~ZenEventData() {
    Dispose(false);
  }

  public void Dispose() {
    Dispose(true);
    global::System.GC.SuppressFinalize(this);
  }

  protected virtual void Dispose(bool disposing) {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          OpenZenPINVOKE.delete_ZenEventData(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public ZenImuData imuData {
    set {
      OpenZenPINVOKE.ZenEventData_imuData_set(swigCPtr, ZenImuData.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = OpenZenPINVOKE.ZenEventData_imuData_get(swigCPtr);
      ZenImuData ret = (cPtr == global::System.IntPtr.Zero) ? null : new ZenImuData(cPtr, false);
      return ret;
    } 
  }

  public ZenGnssData gnssData {
    set {
      OpenZenPINVOKE.ZenEventData_gnssData_set(swigCPtr, ZenGnssData.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = OpenZenPINVOKE.ZenEventData_gnssData_get(swigCPtr);
      ZenGnssData ret = (cPtr == global::System.IntPtr.Zero) ? null : new ZenGnssData(cPtr, false);
      return ret;
    } 
  }

  public ZenEventData_SensorDisconnected sensorDisconnected {
    set {
      OpenZenPINVOKE.ZenEventData_sensorDisconnected_set(swigCPtr, ZenEventData_SensorDisconnected.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = OpenZenPINVOKE.ZenEventData_sensorDisconnected_get(swigCPtr);
      ZenEventData_SensorDisconnected ret = (cPtr == global::System.IntPtr.Zero) ? null : new ZenEventData_SensorDisconnected(cPtr, false);
      return ret;
    } 
  }

  public ZenSensorDesc sensorFound {
    set {
      OpenZenPINVOKE.ZenEventData_sensorFound_set(swigCPtr, ZenSensorDesc.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = OpenZenPINVOKE.ZenEventData_sensorFound_get(swigCPtr);
      ZenSensorDesc ret = (cPtr == global::System.IntPtr.Zero) ? null : new ZenSensorDesc(cPtr, false);
      return ret;
    } 
  }

  public ZenEventData_SensorListingProgress sensorListingProgress {
    set {
      OpenZenPINVOKE.ZenEventData_sensorListingProgress_set(swigCPtr, ZenEventData_SensorListingProgress.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = OpenZenPINVOKE.ZenEventData_sensorListingProgress_get(swigCPtr);
      ZenEventData_SensorListingProgress ret = (cPtr == global::System.IntPtr.Zero) ? null : new ZenEventData_SensorListingProgress(cPtr, false);
      return ret;
    } 
  }

  public ZenEventData() : this(OpenZenPINVOKE.new_ZenEventData(), true) {
  }

}
