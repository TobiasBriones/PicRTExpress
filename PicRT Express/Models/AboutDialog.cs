using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Storage;
using Windows.System;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace PicRT_Express.Models
{
    public sealed partial class AboutDialog : ContentDialog
    {
        const string ThirdPartyLicesesFolderName = "ms-appx:///ThirdPartyLicenses/";
        bool currentScreenStateWide;

        public AboutDialog()
        {
            currentScreenStateWide = true;

            InitializeComponent();
            fix(((Frame) Window.Current.Content).ActualWidth);
            SizeChanged += new SizeChangedEventHandler(OnSizeChanged);
        }

        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            fix(e.NewSize.Width);
        }

        private async void OpenLicenseOpenCVClick(object sender, RoutedEventArgs e)
        {
            const string licenseFileName = "OpenCV LICENSE.txt";
            var resourceLoader = ResourceLoader.GetForCurrentView();
            StorageFile licenseFile = await StorageFile.GetFileFromApplicationUriAsync(new Uri(ThirdPartyLicesesFolderName + licenseFileName));
            string text = await FileIO.ReadTextAsync(licenseFile);
            ContentDialog dialog = new TextDialog
            {
                Title = resourceLoader.GetString("OpenCVOfflineLicenseTitle"),
                Text = text,
            };

            dialog.Closed += async (ContentDialog s, ContentDialogClosedEventArgs a) => await new AboutDialog().ShowAsync();
            Hide();
            await dialog.ShowAsync();
        }

        private void ActionOkClick(object sender, RoutedEventArgs e)
        {
            Hide();
        }

        private void fix(double width)
        {
            if (width < 800 && currentScreenStateWide)
            {
                MainGrid.Width = 400;
                AppLogoCol.Width = new GridLength(0);
                currentScreenStateWide = false;
            }
            else if (width >= 800 && !currentScreenStateWide)
            {
                MainGrid.Width = 600;
                AppLogoCol.Width = new GridLength(150);
                currentScreenStateWide = true;
            }
        }
    }
}
