﻿using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IBluetoothService
    {
        Task StartScanning();

        Task<bool> ConnectToDeviceAsync(IDevice btDevice);

        Task<bool> WriteToCharacteristic();

        Task<string> ReadFromCharacteristic();

        Action<object, IDevice> DeviceFounded { get; set; }
    }
}