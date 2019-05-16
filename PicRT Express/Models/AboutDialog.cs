using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace PicRT_Express.Models
{
    public sealed partial class AboutDialog : ContentDialog
    {
        public AboutDialog()
        {
            InitializeComponent();
        }

        private void ActionOkClick(object sender, RoutedEventArgs e)
        {
            Hide();
        }
    }
}
