﻿using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
using System;
using System.Windows.Input;
using Xamarin.Forms;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;

namespace RETIRODE_APP.ViewModels
{
    public class BluetoothViewModel : BaseViewModel
    {
        private BLEDevice _selectedDevice;

        public IRangeMeasurementService rangeMeasurementService;
        public ObservableCollection<BLEDevice> Devices { get; }
        public Command LoadDevicesCommand { get; }
        public Command<BLEDevice> DeviceTapped { get; }

        public ObservableCollection<BLEDevice> TestDevices { get; }

        public BluetoothViewModel()
        {
            rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            Title = "Bluetooth";
            Devices = new ObservableCollection<BLEDevice>();
            LoadDevicesCommand = new Command(async () => await ExecuteLoadDevicesCommand());
            DeviceTapped = new Command<BLEDevice>(OnDeviceSelected);
            TestDevices = new ObservableCollection<BLEDevice>();

            TestDevices.Add(new BLEDevice
            {
                Name = "device1",
                Identifier = new Guid()
            });
            TestDevices.Add(new BLEDevice
            {
                Name = "device2",
                Identifier = new Guid()
            });
        }

        public ICommand OpenWebCommand { get; }

        private async Task ExecuteLoadDevicesCommand()
        {
            IsBusy = true;

            try
            {
                Devices.Clear();
                //commented just for demo
                //await rangeMeasurementService.StartScanning();
                //var devices = rangeMeasurementService.AvailableDevices;
                foreach (var device in TestDevices)
                {
                    Devices.Add(device);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
            finally
            {
                IsBusy = false;
            }
        }

        public void OnAppearing()
        {
            IsBusy = true;
            SelectedDevice = null;
        }

        public BLEDevice SelectedDevice
        {
            get => _selectedDevice;
            set
            {
                SetProperty(ref _selectedDevice, value);
                OnDeviceSelected(value);
            }
        }

        public void OnDeviceSelected(BLEDevice device)
        {
            if (device == null)
                return;

            rangeMeasurementService.ConnectToRSL10(device);
        }
    }
}