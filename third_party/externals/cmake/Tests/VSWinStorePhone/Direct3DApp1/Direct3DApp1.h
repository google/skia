#pragma once

#include "pch.h"
#include "CubeRenderer.h"

ref class Direct3DApp1 sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
  Direct3DApp1();

  // IFrameworkView Methods.
  virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
  virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
  virtual void Load(Platform::String^ entryPoint);
  virtual void Run();
  virtual void Uninitialize();

protected:
  // Event Handlers.
  void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
  void OnLogicalDpiChanged(Platform::Object^ sender);
  void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
  void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
  void OnResuming(Platform::Object^ sender, Platform::Object^ args);
  void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
  void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
  void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
  void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

private:
  CubeRenderer^ m_renderer;
  bool m_windowClosed;
  bool m_windowVisible;
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
  virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
