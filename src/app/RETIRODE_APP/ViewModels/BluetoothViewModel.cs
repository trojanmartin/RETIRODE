﻿using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using RETIRODE_APP.Views;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        BLEDevice selectedDevice;
        public IRangeMeasurementService rangeMeasurementService;
        public ObservableCollection<BLEDevice> Devices { get; }
        public ICommand LoadDevicesCommand { get; }
        public IAsyncCommand<BLEDevice> DeviceTapped { get; }


        //Test data to be deleted
        public IList<BLEDevice> TestDevices { get; }

        public BluetoothViewModel()
        {
            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            Title = "Bluetooth";
            Devices = new ObservableCollection<BLEDevice>();
            LoadDevicesCommand = new AsyncCommand(async () => await ExecuteLoadDevicesCommand());
            DeviceTapped = new AsyncCommand<BLEDevice>(async (device) => await OnDeviceSelected(device));

            rangeMeasurementService.DeviceDiscoveredEvent -= RangeMeasurementService_DeviceDiscoveredEvent;
            rangeMeasurementService.DeviceDiscoveredEvent += RangeMeasurementService_DeviceDiscoveredEvent;

        }

        // On device discoved add to device list
        private void RangeMeasurementService_DeviceDiscoveredEvent(BLEDevice device)
        {
            Devices.Add(device);
        }

        // On load device command, start scan
        private async Task ExecuteLoadDevicesCommand()
        {
            await EnsureBluetoothEnabled();
            try
            {
                Devices.Clear();
                ShowBusy(() => Devices.Count > 0, 5);
                await rangeMeasurementService.StartScanning();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
        }

        public void OnAppearing()
        {
            IsBusy = true;
            SelectedDevice = null;
        }

        public BLEDevice SelectedDevice
        {
            get => selectedDevice;
            set
            {
                selectedDevice = value;
                SetProperty(ref selectedDevice, value);
            }
        }

        // On bluetooth device clicked, stop scan, connect and redirect to settings page
        // on error display alert
        public async Task OnDeviceSelected(BLEDevice device)
        {
            if (device == null)
                return;

            try
            {
                await rangeMeasurementService.StopScanning();

                await WithBusy(() => rangeMeasurementService.ConnectToRSL10(device));
                var settingPage = TinyIoCContainer.Current.Resolve<SettingsPage>();
                await Application.Current.MainPage.Navigation.PushAsync(settingPage);

                App.IsConnected = true;
            }
            catch (Exception ex)
            {
                await ShowError($"Connecting to {device.Name} failed");
            }            
        }

    }
}
