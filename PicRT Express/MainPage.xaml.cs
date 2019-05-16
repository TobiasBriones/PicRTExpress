using PicRT_Express.Models;
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Graphics.Imaging;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.Storage.Streams;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

namespace PicRT_Express
{

    public sealed partial class MainPage : Page
    {
        private readonly ObservableCollection<string> DefaultImages;
        private readonly ObservableCollection<Effect> Effects;
        private SoftwareBitmap SelectedBitmap;
        private SoftwareBitmap OutputBitmap;
        private StorageFile EditingImageFile;
        private Grid OpenedEffectPropertiesGrid;
        private Effect OpenedEffect;
        private int LastConvolution;
        private int LastPlotFunction;

        public MainPage()
        {
            DefaultImages = new ObservableCollection<string>();
            Effects = new ObservableCollection<Effect>();
            SelectedBitmap = null;
            OutputBitmap = null;
            EditingImageFile = null;
            OpenedEffectPropertiesGrid = null;
            OpenedEffect = null;
            LastConvolution = 0;
            LastPlotFunction = 0;

            InitializeComponent();
            SizeChanged += new SizeChangedEventHandler(OnSizeChanged);

        }

        /* ---------------------------------------- BASE METHODS ---------------------------------------- */
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await LoadDefaultImages();
            LoadEffects();
        }

        /* ---------------------------------------- PRIVATE METHODS ---------------------------------------- */
        private void LoadEffects()
        {
            var resourceLoader = ResourceLoader.GetForCurrentView();

            Effects.Add(new Effect(Effect.BRIGHTNESS, resourceLoader.GetString("EffectBrightness")));
            Effects.Add(new Effect(Effect.CONVOLUTION, resourceLoader.GetString("EffectConvolution")));
            Effects.Add(new Effect(Effect.FLIP, resourceLoader.GetString("EffectFlip")));
            Effects.Add(new Effect(Effect.GRAY, resourceLoader.GetString("EffectGray")));
            Effects.Add(new Effect(Effect.RANDOM_LINES, resourceLoader.GetString("EffectRandomLines")));
            Effects.Add(new Effect(Effect.RGB, resourceLoader.GetString("EffectRGB")));
            Effects.Add(new Effect(Effect.ROTATE, resourceLoader.GetString("EffectRotate")));
            Effects.Add(new Effect(Effect.SEPIA, resourceLoader.GetString("EffectSepia")));
            Effects.Add(new Effect(Effect.PIXELATED, resourceLoader.GetString("EffectPixelated")));
            Effects.Add(new Effect(Effect.PLOT, resourceLoader.GetString("EffectPlot")));
        }

        private void ShowProgressBar(string strRes)
        {
            var resourceLoader = ResourceLoader.GetForCurrentView();
            var message = resourceLoader.GetString(strRes);
            FlyoutNotification.Opacity = 100;

            FlyoutText.Text = message;
        }

        private void HideProgressBar()
        {
            FlyoutNotification.Opacity = 0;
        }

        private void OpenEffectPropertiesPanel()
        {
            var id = OpenedEffect.getId();

            // Close current properties panel if there is any
            if (OpenedEffectPropertiesGrid != null)
            {
                OpenedEffectPropertiesGrid.Visibility = Visibility.Collapsed;
            }

            if (id == Effect.FLIP || id == Effect.GRAY || id == Effect.SEPIA)
            {
                if (OpenedEffectPropertiesGrid != null)
                {
                    OpenedEffectPropertiesGrid.Visibility = Visibility.Collapsed;
                }
                OpenedEffectPropertiesGrid = null;
            }
            else if (id == Effect.BRIGHTNESS)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesBrightnessGrid;
            }
            else if(id == Effect.CONVOLUTION)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesConvolutionGrid;
                LastConvolution = 0;
            }
            else if (id == Effect.RANDOM_LINES)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesRandomLinesGrid;
            }
            else if (id == Effect.RGB)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesRGBGrid;
            }
            else if (id == Effect.ROTATE)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesRotateGrid;
            }
            else if (id == Effect.PIXELATED)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesPixelateGrid;
                LastPlotFunction = 0;
            }
            else if (id == Effect.PLOT)
            {
                OpenedEffectPropertiesGrid = EffectPropertiesPlotGrid;
            }

            // Open property panel if needed
            if (OpenedEffectPropertiesGrid != null)
            {
                OpenedEffectPropertiesGrid.Visibility = Visibility.Visible;
            }
        }

        private int GetIntValueFromTextBox(TextBox box, int def = 0)
        {
            int value;

            try
            {
                value = int.Parse(box.Text);
            }
            catch (FormatException)
            {
                value = def;
            }
            return value;
        }

        private double GetDoubleValueFromTextBox(TextBox box, double def = 0)
        {
            double value;

            try
            {
                value = double.Parse(box.Text);
            }
            catch (FormatException)
            {
                value = def;
            }
            return value;
        }

        /* ---------------------------------------- PRIVATE METHODS (ASYNC) ---------------------------------------- */
        private async Task LoadDefaultImages()
        {
            var imagesDirectory = await Windows.ApplicationModel.Package.Current.InstalledLocation.GetFolderAsync(@"Assets\Images");
            var files = await imagesDirectory.GetFilesAsync();

            foreach (StorageFile file in files)
            {
                DefaultImages.Add(file.Path);
            }
        }

        private async Task SetImageAsync(StorageFile file)
        {
            ShowProgressBar("FlyoutLoading");
            using (IRandomAccessStream stream = await file.OpenAsync(FileAccessMode.Read))
            {
                BitmapDecoder decoder = await BitmapDecoder.CreateAsync(stream);
                SelectedBitmap = await decoder.GetSoftwareBitmapAsync();
            }

            if (SelectedBitmap.BitmapPixelFormat != BitmapPixelFormat.Bgra8 || SelectedBitmap.BitmapAlphaMode != BitmapAlphaMode.Premultiplied)
            {
                SelectedBitmap = SoftwareBitmap.Convert(SelectedBitmap, BitmapPixelFormat.Bgra8, BitmapAlphaMode.Premultiplied);
            }
            // Output is a copy of Selected and the source is the same for the two views
            var bitmapSource = new SoftwareBitmapSource();

            await bitmapSource.SetBitmapAsync(SelectedBitmap);

            OutputBitmap = SoftwareBitmap.Copy(SelectedBitmap);
            EditingImageFile = file;
            SelectedImage.Source = bitmapSource;

            await bitmapSource.SetBitmapAsync(OutputBitmap);
            ResultImage.Source = bitmapSource;
            SaveButton.IsEnabled = true;
            ResetButton.IsEnabled = false;
            EffectActionsGrid.Visibility = Visibility.Visible;
            HideProgressBar();
        }

        private async Task SetImageAsync(string path)
        {
            StorageFile file = await StorageFile.GetFileFromPathAsync(path);

            await SetImageAsync(file);
        }

        private async void CommandInvokedHandlerAsync(IUICommand command)
        {
            if (command.Id.Equals(0))
            {
                await SetImageAsync(EditingImageFile);
            }
        }

        private async Task ApplyEffect()
        {
            ShowProgressBar("FlyoutLoading");
            var id = OpenedEffect.getId();
            var helper = new OpenCVBridge.OpenCVHelper();
            var bitmapSource = new SoftwareBitmapSource();
            OutputBitmap = new SoftwareBitmap(SelectedBitmap.BitmapPixelFormat, SelectedBitmap.PixelWidth, SelectedBitmap.PixelHeight, BitmapAlphaMode.Premultiplied);

            // BUG saves black image to storage in some implementations //
            if (id == Effect.BRIGHTNESS)
            {
                double brightness = GetDoubleValueFromTextBox(EffectBrightnessAmountBox) / 100;

                await Task.Run(() => helper.Brightness(SelectedBitmap, OutputBitmap, brightness));
            }
            else if (id == Effect.CONVOLUTION)
            {
                await Task.Run(() => helper.Convolution(SelectedBitmap, OutputBitmap, LastConvolution));
            }
            else if (id == Effect.FLIP)
            {
                await Task.Run(() => helper.Flip(SelectedBitmap, OutputBitmap));
            }
            else if(id == Effect.GRAY)
            {
                await Task.Run(() => helper.Gray(SelectedBitmap, OutputBitmap));
            }
            else if (id == Effect.RANDOM_LINES)
            {
                int horizontalSpace = GetIntValueFromTextBox(EffectRandomLinesHorizontalSpaceBox);
                int verticalSpace = GetIntValueFromTextBox(EffectRandomLinesVerticalSpaceBox);

                await Task.Run(() => helper.RandomLines(SelectedBitmap, OutputBitmap, horizontalSpace, verticalSpace));
            }
            else if (id == Effect.RGB)
            {
                double r = GetDoubleValueFromTextBox(EffectRGB_RBox, 100) / 100;
                double g = GetDoubleValueFromTextBox(EffectRGB_GBox, 100) / 100;
                double b = GetDoubleValueFromTextBox(EffectRGB_BBox, 100) / 100;

                await Task.Run(() => helper.RGBComponents(SelectedBitmap, OutputBitmap, r, g, b));
            }
            else if (id == Effect.ROTATE)
            {
                double angle = GetIntValueFromTextBox(EffectRotateAngleBox, 90) * Math.PI / 180;
                
                await Task.Run(() => helper.Rotate(SelectedBitmap, OutputBitmap, angle));
            }
            else if (id == Effect.SEPIA)
            {
                await Task.Run(() => helper.Sepia(SelectedBitmap, OutputBitmap));
            }
            else if (id == Effect.PIXELATED)
            {
                int radius = GetIntValueFromTextBox(EffectPixelateRadiusBox);

                await Task.Run(() => helper.Pixelate(SelectedBitmap, OutputBitmap, radius));
            }
            else if (id == Effect.PLOT)
            {
                int precision = GetIntValueFromTextBox(EffectPlotPrecisionBox);
                double scaleX = GetDoubleValueFromTextBox(EffectPlotScaleXBox, 1);
                double scaleY = GetDoubleValueFromTextBox(EffectPlotScaleYBox, 1);

                await Task.Run(() => helper.Plot(SelectedBitmap, OutputBitmap, LastPlotFunction, precision, scaleX, scaleY));
            }
            await bitmapSource.SetBitmapAsync(OutputBitmap);
            ResultImage.Source = bitmapSource;
            ResetButton.IsEnabled = true;
            HideProgressBar();
        }

        private async void SaveSoftwareBitmapToFile(SoftwareBitmap softwareBitmap, StorageFile outputFile)
        {
            using (IRandomAccessStream stream = await outputFile.OpenAsync(FileAccessMode.ReadWrite))
            {
                BitmapEncoder encoder = await BitmapEncoder.CreateAsync(BitmapEncoder.JpegEncoderId, stream);

                encoder.SetSoftwareBitmap(softwareBitmap);
                encoder.BitmapTransform.ScaledWidth = (uint)softwareBitmap.PixelWidth;
                encoder.BitmapTransform.ScaledHeight = (uint)softwareBitmap.PixelHeight;
                encoder.BitmapTransform.InterpolationMode = BitmapInterpolationMode.Fant;
                encoder.IsThumbnailGenerated = true;

                try
                {
                    await encoder.FlushAsync();
                }
                catch (Exception err)
                {
                    const int WINCODEC_ERR_UNSUPPORTEDOPERATION = unchecked((int)0x88982F81);
                    switch (err.HResult)
                    {
                        case WINCODEC_ERR_UNSUPPORTEDOPERATION:
                            // If the encoder does not support writing a thumbnail, then try again
                            // but disable thumbnail generation.
                            encoder.IsThumbnailGenerated = false;
                            break;

                        default:
                            throw;
                    }
                }
                if (encoder.IsThumbnailGenerated == false)
                {
                    await encoder.FlushAsync();
                }
            }
        }

        /* ---------------------------------------- EVENTS ---------------------------------------- */
        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            var width = ActualWidth;
            var imagesGridPanel = (ItemsWrapGrid)DefaultImagesGrid.ItemsPanelRoot;
            var imagesGridWidth = DefaultImagesGrid.ActualWidth;
            var imagesGridColumns = 6;
            var effectsGridPanel = (ItemsWrapGrid)EffectsGrid.ItemsPanelRoot;
            var effectsGridWidth = EffectsGrid.ActualWidth;
            var effectsGridColumns = 6;

            // Wide: 960px, XWide: 2560px
            if (width < 600)
            {
                imagesGridColumns = effectsGridColumns = 2;
            }
            else if (width < 960)
            {
                imagesGridColumns = 4;
                effectsGridColumns = 3;
            }
            else if (width < 1360)
            {
                imagesGridColumns = 2;
                effectsGridColumns = 3;
            }
            else if (width < 1920)
            {
                imagesGridColumns = 4;
                effectsGridColumns = 4;
            }
            imagesGridPanel.ItemWidth = imagesGridPanel.ItemHeight = imagesGridWidth / imagesGridColumns;
            effectsGridPanel.ItemWidth = effectsGridWidth / effectsGridColumns;
            effectsGridPanel.ItemHeight = 60;
        }

        private async void DefaultImageSelected(object sender, ItemClickEventArgs e)
        {
            string path = e.ClickedItem.ToString();

            await SetImageAsync(path);
        }

        private async void SelectImageButtonClick(object sender, RoutedEventArgs e)
        {
            var picker = new FileOpenPicker
            {
                ViewMode = PickerViewMode.Thumbnail,
                SuggestedStartLocation = PickerLocationId.PicturesLibrary
            };

            picker.FileTypeFilter.Add(".jpg");
            picker.FileTypeFilter.Add(".jpeg");
            picker.FileTypeFilter.Add(".png");

            StorageFile file = await picker.PickSingleFileAsync();

            if (file != null)
            {
                await SetImageAsync(file);
            }
        }

        private async void EffectsGridItemClick(object sender, ItemClickEventArgs e)
        {
            if (SelectedBitmap == null) return;
            OpenedEffect = e.ClickedItem as Effect;

            await ApplyEffect();
            OpenEffectPropertiesPanel();
        }

        private async void SaveButtonClick(object sender, RoutedEventArgs e)
        {
            StorageFolder rootFolder = ApplicationData.Current.LocalFolder;
            var projectFolderName = "SavedImages";
            StorageFolder projectFolder = await rootFolder.CreateFolderAsync(projectFolderName, CreationCollisionOption.OpenIfExists);
            var file = await projectFolder.CreateFileAsync(EditingImageFile.Name, CreationCollisionOption.GenerateUniqueName);

            SaveSoftwareBitmapToFile(OutputBitmap, file);
        }

        private async void ResetButtonClick(object sender, RoutedEventArgs e)
        {
            var resourceLoader = ResourceLoader.GetForCurrentView();
            var msg = resourceLoader.GetString("ResetDialog");
            var messageDialog = new MessageDialog(msg);

            messageDialog.Commands.Add(new UICommand(resourceLoader.GetString("CancelAction"), new UICommandInvokedHandler(CommandInvokedHandlerAsync), 1));
            messageDialog.Commands.Add(new UICommand(resourceLoader.GetString("ResetAction"), new UICommandInvokedHandler(CommandInvokedHandlerAsync), 0));

            messageDialog.DefaultCommandIndex = 0;
            messageDialog.CancelCommandIndex = 1;

            await messageDialog.ShowAsync();
        }

        private async void OpenImagesButtonClick(object sender, RoutedEventArgs e)
        {
            StorageFolder rootFolder = ApplicationData.Current.LocalFolder;
            var projectFolderName = "SavedImages";
            StorageFolder projectFolder = await rootFolder.CreateFolderAsync(projectFolderName, CreationCollisionOption.OpenIfExists);

            await Windows.System.Launcher.LaunchFolderAsync(projectFolder);
        }

        private async void SaveChangeButtonClick(object sender, RoutedEventArgs e)
        {
            var bitmapSource = new SoftwareBitmapSource();

            await bitmapSource.SetBitmapAsync(OutputBitmap);

            SelectedBitmap = SoftwareBitmap.Copy(OutputBitmap);
            SelectedImage.Source = bitmapSource;
        }

        private async void ApplyEffectButtonClick(object sender, RoutedEventArgs e)
        {
            if (OpenedEffect != null)
            {
                await ApplyEffect();
            }
        }

        // Effect Properties Events //
        private async void EffectConvolutionButton(object sender, RoutedEventArgs e)
        {
            LastConvolution = int.Parse(((Button) sender).Tag.ToString());

            await ApplyEffect();
        }

        private async void EffectPlotButton(object sender, RoutedEventArgs e)
        {
            LastPlotFunction = int.Parse(((Button)sender).Tag.ToString());

            await ApplyEffect();
        }

        // CommandBar Events //
        private async void AboutCommandBarButtonClick(object sender, RoutedEventArgs e)
        {
            var aboutDialog = new AboutDialog();

            await aboutDialog.ShowAsync();

        }

    }

}
