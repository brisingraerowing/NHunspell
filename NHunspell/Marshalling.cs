// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Marshalling.cs" company="Maierhofer Software and the Hunspell Developers">
//   (c) by Maierhofer Software an the Hunspell Developers
// </copyright>
// <summary>
//   The marshal hunspell dll.
// </summary>
// --------------------------------------------------------------------------------------------------------------------

namespace NHunspell
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;

    /// <summary>
    ///   The marshal hunspell dll.
    /// </summary>
    internal static class MarshalHunspellDll
    {
		private static DllLoadUtils dllLoadUtils;

        #region Static Fields

        /// <summary>
        ///   The hunspell add.
        /// </summary>
        internal static HunspellAddDelegate HunspellAdd;

        /// <summary>
        ///   The hunspell add with affix.
        /// </summary>
        internal static HunspellAddWithAffixDelegate HunspellAddWithAffix;

        /// <summary>
        ///   The hunspell analyze.
        /// </summary>
        internal static HunspellAnalyzeDelegate HunspellAnalyze;

        /// <summary>
        ///   The hunspell free.
        /// </summary>
        internal static HunspellFreeDelegate HunspellFree;

        /// <summary>
        ///   The hunspell generate.
        /// </summary>
        internal static HunspellGenerateDelegate HunspellGenerate;

        /// <summary>
        ///   The hunspell init.
        /// </summary>
        internal static HunspellInitDelegate HunspellInit;

        /// <summary>
        /// The hunspell remove.
        /// </summary>
        internal static HunspellRemoveDelegate HunspellRemove;

        /// <summary>
        ///   The hunspell spell.
        /// </summary>
        internal static HunspellSpellDelegate HunspellSpell;

        /// <summary>
        ///   The hunspell stem.
        /// </summary>
        internal static HunspellStemDelegate HunspellStem;

        /// <summary>
        ///   The hunspell suggest.
        /// </summary>
        internal static HunspellSuggestDelegate HunspellSuggest;

        /// <summary>
        ///   The hyphen free.
        /// </summary>
        internal static HyphenFreeDelegate HyphenFree;

        /// <summary>
        ///   The hyphen hyphenate.
        /// </summary>
        internal static HyphenHyphenateDelegate HyphenHyphenate;

        /// <summary>
        ///   The hyphen init.
        /// </summary>
        internal static HyphenInitDelegate HyphenInit;

        /// <summary>
        /// The native dll reference count lock.
        /// </summary>
        private static readonly object nativeDllReferenceCountLock = new object();

        /// <summary>
        ///   The dll handle.
        /// </summary>
        private static IntPtr dllHandle = IntPtr.Zero;

        /// <summary>
        ///   The native dll path.
        /// </summary>
        private static string nativeDLLPath;

        /// <summary>
        /// The native dll reference count.
        /// </summary>
        private static int nativeDllReferenceCount;

        #endregion

        // Hunspell
        #region Delegates

        /// <summary>
        ///   The hunspell add delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate bool HunspellAddDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hunspell add with affix delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        /// <param name="example"> The example. </param>
        internal delegate bool HunspellAddWithAffixDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word, [MarshalAs(UnmanagedType.LPWStr)] string example);

        /// <summary>
        ///   The hunspell analyze delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate IntPtr HunspellAnalyzeDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hunspell free delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        internal delegate void HunspellFreeDelegate(IntPtr handle);

        /// <summary>
        ///   The hunspell generate delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        /// <param name="word2"> The word 2. </param>
        internal delegate IntPtr HunspellGenerateDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word, [MarshalAs(UnmanagedType.LPWStr)] string word2);

        /// <summary>
        ///   The hunspell init delegate.
        /// </summary>
        /// <param name="affixData"> The affix data. </param>
        /// <param name="affixDataSize"> The affix data size. </param>
        /// <param name="dictionaryData"> The dictionary data. </param>
        /// <param name="dictionaryDataSize"> The dictionary data size. </param>
        /// <param name="key"> The key. </param>
        internal delegate IntPtr HunspellInitDelegate([MarshalAs(UnmanagedType.LPArray)] byte[] affixData, IntPtr affixDataSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dictionaryData, IntPtr dictionaryDataSize, string key);

        /// <summary>
        /// The hunspell remove delegate.
        /// </summary>
        /// <param name="handle">
        /// The handle.
        /// </param>
        /// <param name="word">
        /// The word.
        /// </param>
        internal delegate bool HunspellRemoveDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hunspell spell delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate bool HunspellSpellDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hunspell stem delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate IntPtr HunspellStemDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hunspell suggest delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate IntPtr HunspellSuggestDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hyphen free delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        internal delegate void HyphenFreeDelegate(IntPtr handle);

        /// <summary>
        ///   The hyphen hyphenate delegate.
        /// </summary>
        /// <param name="handle"> The handle. </param>
        /// <param name="word"> The word. </param>
        internal delegate IntPtr HyphenHyphenateDelegate(IntPtr handle, [MarshalAs(UnmanagedType.LPWStr)] string word);

        /// <summary>
        ///   The hyphen init delegate.
        /// </summary>
        /// <param name="dictData"> The dict data. </param>
        /// <param name="dictDataSize"> The dict data size. </param>
        internal delegate IntPtr HyphenInitDelegate([MarshalAs(UnmanagedType.LPArray)] byte[] dictData, IntPtr dictDataSize);

        #endregion

        #region Properties

        /// <summary>
        ///   Gets or sets NativeDLLPath.
        /// </summary>
        /// <exception cref="InvalidOperationException"></exception>
        internal static string NativeDLLPath
        {
            get
            {
                if (nativeDLLPath == null)
                {
                    nativeDLLPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, AppDomain.CurrentDomain.RelativeSearchPath ?? string.Empty);
                }

                return nativeDLLPath;
            }

            set
            {
                if (dllHandle != IntPtr.Zero)
                {
                    throw new InvalidOperationException("Native Library is already loaded");
                }

                nativeDLLPath = value;
            }
        }

        #endregion

        #region Methods

        /// <summary>
        ///   References the native hunspell DLL.
        /// </summary>
        /// <exception cref="System.DllNotFoundException"></exception>
        /// <exception cref="System.NotSupportedException"></exception>
        internal static void ReferenceNativeHunspellDll()
        {
            lock (nativeDllReferenceCountLock)
            {
                if (nativeDllReferenceCount == 0)
                {
					bool success = false;

                    if (dllHandle != IntPtr.Zero)
                    {
                        throw new InvalidOperationException("Native Dll handle is not Zero");
                    }

                    try
                    {
						string dllName;
						string dllPath;

						if (Environment.OSVersion.Platform == PlatformID.Win32NT)
						{
							dllLoadUtils = new DllLoadUtilsWindows();
							if (IntPtr.Size == 4)
								dllName = Resources.HunspellX86DllName;
							else
								dllName = Resources.HunspellX64DllName;
						}
						else
						{
							dllLoadUtils = new DllLoadUtilsUnix();
							dllName = "libhunspell.dylib";
						}

						dllPath = Path.Combine(NativeDLLPath, dllName);
						dllHandle = dllLoadUtils.LoadLibrary(Path.Combine(NativeDLLPath, dllPath));
                        if (dllHandle == IntPtr.Zero)
                        {
							if (IntPtr.Size == 4)
								throw new DllNotFoundException(string.Format(Resources.HunspellX86DllNotFoundMessage, dllPath));
							else
								throw new DllNotFoundException(string.Format(Resources.HunspellX64DllNotFoundMessage, dllPath));
                        }

                        HunspellInit = (HunspellInitDelegate)GetDelegate("HunspellInit", typeof(HunspellInitDelegate));
                        HunspellFree = (HunspellFreeDelegate)GetDelegate("HunspellFree", typeof(HunspellFreeDelegate));

                        HunspellAdd = (HunspellAddDelegate)GetDelegate("HunspellAdd", typeof(HunspellAddDelegate));
                        HunspellAddWithAffix = (HunspellAddWithAffixDelegate)GetDelegate("HunspellAddWithAffix", typeof(HunspellAddWithAffixDelegate));

                        HunspellRemove = (HunspellRemoveDelegate)GetDelegate("HunspellRemove", typeof(HunspellRemoveDelegate));

                        HunspellSpell = (HunspellSpellDelegate)GetDelegate("HunspellSpell", typeof(HunspellSpellDelegate));
                        HunspellSuggest = (HunspellSuggestDelegate)GetDelegate("HunspellSuggest", typeof(HunspellSuggestDelegate));

                        HunspellAnalyze = (HunspellAnalyzeDelegate)GetDelegate("HunspellAnalyze", typeof(HunspellAnalyzeDelegate));
                        HunspellStem = (HunspellStemDelegate)GetDelegate("HunspellStem", typeof(HunspellStemDelegate));
                        HunspellGenerate = (HunspellGenerateDelegate)GetDelegate("HunspellGenerate", typeof(HunspellGenerateDelegate));

                        HyphenInit = (HyphenInitDelegate)GetDelegate("HyphenInit", typeof(HyphenInitDelegate));
                        HyphenFree = (HyphenFreeDelegate)GetDelegate("HyphenFree", typeof(HyphenFreeDelegate));
                        HyphenHyphenate = (HyphenHyphenateDelegate)GetDelegate("HyphenHyphenate", typeof(HyphenHyphenateDelegate));

						success = true;
                    }
					finally
                    {
						if (dllHandle != IntPtr.Zero && !success)
                        {
                            dllLoadUtils.FreeLibrary(dllHandle);
                            dllHandle = IntPtr.Zero;
                        }
                    }
                }

                ++nativeDllReferenceCount;
            }
        }

        /// <summary>
        /// The un reference native hunspell dll.
        /// </summary>
        /// <exception cref="InvalidOperationException">
        /// </exception>
        internal static void UnReferenceNativeHunspellDll()
        {
            lock (nativeDllReferenceCountLock)
            {
                if (nativeDllReferenceCount <= 0)
                {
                    throw new InvalidOperationException("native DLL reference count is <= 0");
                }

                --nativeDllReferenceCount;

                if (nativeDllReferenceCount == 0)
                {
                    if (dllHandle == IntPtr.Zero)
                    {
                        throw new InvalidOperationException("Native DLL handle is Zero");
                    }

					dllLoadUtils.FreeLibrary(dllHandle);
                    dllHandle = IntPtr.Zero;
                }
            }
        }

        /// <summary>
        /// The get delegate.
        /// </summary>
        /// <param name="procName">
        /// The proc name. 
        /// </param>
        /// <param name="delegateType">
        /// The delegate type. 
        /// </param>
        /// <returns>
        /// The <see cref="Delegate"/>.
        /// </returns>
        /// <exception cref="EntryPointNotFoundException">
        /// </exception>
        private static Delegate GetDelegate(string procName, Type delegateType)
        {
			IntPtr procAdress = dllLoadUtils.GetProcAddress(dllHandle, procName);
            if (procAdress == IntPtr.Zero)
            {
                throw new EntryPointNotFoundException("Function: " + procName);
            }

            return Marshal.GetDelegateForFunctionPointer(procAdress, delegateType);
        }

        #endregion

		interface DllLoadUtils
		{
			IntPtr LoadLibrary(string fileName);
			void FreeLibrary(IntPtr handle);
			IntPtr GetProcAddress(IntPtr dllHandle, string name);
		}

		public class DllLoadUtilsWindows : DllLoadUtils
		{
			void DllLoadUtils.FreeLibrary(IntPtr handle)
			{
				FreeLibrary(handle);
			}

			IntPtr DllLoadUtils.GetProcAddress(IntPtr dllHandle, string name)
			{
				return GetProcAddress(dllHandle, name);
			}

			IntPtr DllLoadUtils.LoadLibrary(string fileName)
			{
				return LoadLibrary(fileName);
			}

			[DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
			private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

			[DllImport("kernel32.dll")]
			private static extern IntPtr LoadLibrary(string fileName);

			[DllImport("kernel32.dll", SetLastError = true)]
			private static extern bool FreeLibrary(IntPtr hModule);
		}

		internal class DllLoadUtilsUnix : DllLoadUtils
		{
			public IntPtr LoadLibrary(string fileName)
			{
				return dlopen(fileName, RTLD_NOW);
			}

			public void FreeLibrary(IntPtr handle)
			{
				dlclose(handle);
			}

			public IntPtr GetProcAddress(IntPtr dllHandle, string name)
			{
				// clear previous errors if any
				dlerror();
				var res = dlsym(dllHandle, name);
				var errPtr = dlerror();
				if (errPtr != IntPtr.Zero) {
					throw new Exception("dlsym: " + Marshal.PtrToStringAnsi(errPtr));
				}
				return res;
			}

			const int RTLD_NOW = 2;

			[DllImport("libdl.dylib")]
			private static extern IntPtr dlopen(String fileName, int flags);

			[DllImport("libdl.dylib")]
			private static extern IntPtr dlsym(IntPtr handle, String symbol);

			[DllImport("libdl.dylib")]
			private static extern int dlclose(IntPtr handle);

			[DllImport("libdl.dylib")]
			private static extern IntPtr dlerror();
		}
    }
}