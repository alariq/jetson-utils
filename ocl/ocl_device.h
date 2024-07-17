#ifndef __CL_DEVICE_H__


class CL_DeviceInfo
	{
	public :
		cl_device_type dType;				/**< dType device type*/
		cl_uint vendorId;					/**< vendorId VendorId of device*/
		cl_uint maxComputeUnits;			/**< maxComputeUnits maxComputeUnits of device*/
		cl_uint maxWorkItemDims;			/**< maxWorkItemDims maxWorkItemDimensions VendorId of device*/
		size_t* maxWorkItemSizes;			/**< maxWorkItemSizes maxWorkItemSizes of device*/
		size_t maxWorkGroupSize;			/**< maxWorkGroupSize max WorkGroup Size of device*/
		cl_uint preferredCharVecWidth;		/**< preferredCharVecWidth preferred Char VecWidth of device*/
		cl_uint preferredShortVecWidth;		/**< preferredShortVecWidth preferred Short VecWidth of device*/
		cl_uint preferredIntVecWidth;		/**< preferredIntVecWidth preferred Int VecWidth of device*/
		cl_uint preferredLongVecWidth;		/**< preferredLongVecWidth preferred Long VecWidth of device*/
		cl_uint preferredFloatVecWidth;		/**< preferredFloatVecWidth preferredFloatVecWidth of device*/
		cl_uint preferredDoubleVecWidth;	/**< preferredDoubleVecWidth preferred Double VecWidth of device*/
		cl_uint preferredHalfVecWidth;		/**< preferredHalfVecWidth preferred Half VecWidth of device*/
		cl_uint nativeCharVecWidth;		 	/**< nativeCharVecWidth native Char VecWidth of device*/
		cl_uint nativeShortVecWidth;		/**< nativeShortVecWidth nativeShortVecWidth of device*/
		cl_uint nativeIntVecWidth;			/**< nativeIntVecWidth nativeIntVecWidth of device*/
		cl_uint nativeLongVecWidth;			/**< nativeLongVecWidth nativeLongVecWidth of device*/
		cl_uint nativeFloatVecWidth;		/**< nativeFloatVecWidth native Float VecWidth of device*/
		cl_uint nativeDoubleVecWidth;		/**< nativeDoubleVecWidth native Double VecWidth of device*/
		cl_uint nativeHalfVecWidth;			/**< nativeHalfVecWidth native Half VecWidth of device*/
		cl_uint maxClockFrequency;			/**< maxClockFrequency max Clock Frequency of device*/
		cl_uint addressBits;				/**< addressBits address Bits of device*/
		cl_ulong maxMemAllocSize;			/**< maxMemAllocSize max Mem Alloc Size of device*/
		cl_bool imageSupport;				/**< imageSupport image Support of device*/
		cl_uint maxReadImageArgs;			/**< maxReadImageArgs max ReadImage Args of device*/
		cl_uint maxWriteImageArgs;			/**< maxWriteImageArgs max Write Image Args of device*/
		size_t image2dMaxWidth;				/**< image2dMaxWidth image 2dMax Width of device*/
		size_t image2dMaxHeight;			/**< image2dMaxHeight image 2dMax Height of device*/
		size_t image3dMaxWidth;				/**< image3dMaxWidth image3d MaxWidth of device*/ 
		size_t image3dMaxHeight;			/**< image3dMaxHeight image 3dMax Height of device*/
		size_t image3dMaxDepth;				/**< image3dMaxDepth image 3dMax Depth of device*/
		size_t maxSamplers;					/**< maxSamplers maxSamplers of device*/
		size_t maxParameterSize;			/**< maxParameterSize maxParameterSize of device*/
		cl_uint memBaseAddressAlign;		/**< memBaseAddressAlign memBase AddressAlign of device*/
		cl_uint minDataTypeAlignSize;		/**< minDataTypeAlignSize minDataType AlignSize of device*/
		cl_device_fp_config singleFpConfig;	/**< singleFpConfig singleFpConfig of device*/
		cl_device_fp_config doubleFpConfig; /**< doubleFpConfig doubleFpConfig of device*/
		cl_device_mem_cache_type globleMemCacheType; /**< globleMemCacheType globleMem CacheType of device*/
		cl_uint globalMemCachelineSize;		/**< globalMemCachelineSize globalMem Cacheline Size of device*/
		cl_ulong globalMemCacheSize;		/**< globalMemCacheSize globalMem CacheSize of device*/
		cl_ulong globalMemSize;				/**< globalMemSize globalMem Size of device*/
		cl_ulong maxConstBufSize;			/**< maxConstBufSize maxConst BufSize of device*/
		cl_uint maxConstArgs;				/**< maxConstArgs max ConstArgs of device*/
		cl_device_local_mem_type localMemType;/**< localMemType local MemType of device*/
		cl_ulong localMemSize;				/**< localMemSize localMem Size of device*/
		cl_bool errCorrectionSupport;		/**< errCorrectionSupport errCorrectionSupport of device*/
		cl_bool hostUnifiedMem;				/**< hostUnifiedMem hostUnifiedMem of device*/
		size_t timerResolution;				/**< timerResolution timerResolution of device*/
		cl_bool endianLittle;				/**< endianLittle endian Little of device*/
		cl_bool available;					/**< available available of device*/
		cl_bool compilerAvailable;			/**< compilerAvailable compilerAvailable of device*/
		cl_device_exec_capabilities execCapabilities;/**< execCapabilities exec Capabilities of device*/
		cl_command_queue_properties queueProperties;/**< queueProperties queueProperties of device*/
		cl_platform_id platform;			/**< platform platform of device*/
		char* name;							/**< name name of device*/
		char* vendorName;					/**< venderName vender Name of device*/
		char* driverVersion;				/**< driverVersion driver Version of device*/
		char* profileType;					/**< profileType profile Type of device*/
		char* deviceVersion;				/**< deviceVersion device Version of device*/
		char* openclCVersion;				/**< openclCVersion opencl C Version of device*/
		char* extensions;					/**< extensions extensions of device*/

		/**
		 * Constructor
		 */
		CL_DeviceInfo()
		{
			dType = CL_DEVICE_TYPE_GPU;
			vendorId = 0;
			maxComputeUnits = 0;
			maxWorkItemDims = 0;
			maxWorkItemSizes = NULL;
			maxWorkGroupSize = 0;
			preferredCharVecWidth = 0;
			preferredShortVecWidth = 0;
			preferredIntVecWidth = 0;
			preferredLongVecWidth = 0;
			preferredFloatVecWidth = 0;
			preferredDoubleVecWidth = 0;
			preferredHalfVecWidth = 0;
			nativeCharVecWidth = 0;
			nativeShortVecWidth = 0;
			nativeIntVecWidth = 0;
			nativeLongVecWidth = 0;
			nativeFloatVecWidth = 0;
			nativeDoubleVecWidth = 0;
			nativeHalfVecWidth = 0;
			maxClockFrequency = 0;
			addressBits = 0;
			maxMemAllocSize = 0;
			imageSupport = CL_FALSE;
			maxReadImageArgs = 0;
			maxWriteImageArgs = 0;
			image2dMaxWidth = 0;
			image2dMaxHeight = 0;
			image3dMaxWidth = 0;
			image3dMaxHeight = 0;
			image3dMaxDepth = 0;
			maxSamplers = 0;
			maxParameterSize = 0;
			memBaseAddressAlign = 0;
			minDataTypeAlignSize = 0;
			singleFpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN;
			doubleFpConfig = CL_FP_FMA |
							 CL_FP_ROUND_TO_NEAREST |
							 CL_FP_ROUND_TO_ZERO |
							 CL_FP_ROUND_TO_INF |
							 CL_FP_INF_NAN |
							 CL_FP_DENORM;
			globleMemCacheType = CL_NONE;
			globalMemCachelineSize = CL_NONE;
			globalMemCacheSize = 0;
			globalMemSize = 0;
			maxConstBufSize = 0;
			maxConstArgs = 0;
			localMemType = CL_LOCAL;
			localMemSize = 0;
			errCorrectionSupport = CL_FALSE;
			hostUnifiedMem = CL_FALSE;
			timerResolution = 0;
			endianLittle = CL_FALSE;
			available = CL_FALSE;
			compilerAvailable = CL_FALSE;
			execCapabilities = CL_EXEC_KERNEL;
			queueProperties = 0;
			platform = 0;
			name = NULL;
			vendorName = NULL;
			driverVersion = NULL;
			profileType = NULL;
			deviceVersion = NULL;
			openclCVersion = NULL;
			extensions = NULL;
		};

		/**
		 * Destructor
		 */
		~CL_DeviceInfo()
		{
			delete maxWorkItemSizes;
			delete name;
			delete vendorName;
			delete driverVersion;
			delete profileType;
			delete deviceVersion;
			delete openclCVersion;
			delete extensions;
		};

		/**
		 * setDeviceInfo
		 * Set all information for a given device id
		 * @param deviceId deviceID 
		 * @return 0 if success else nonzero
		 */
		bool setDeviceInfo(cl_device_id deviceId);
	};

#endif // __CL_DEVICE_H__
