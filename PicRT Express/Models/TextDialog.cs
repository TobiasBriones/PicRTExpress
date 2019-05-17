using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace PicRT_Express.Models
{
    public sealed partial class TextDialog : ContentDialog
    {
        public string Text
        {
            set
            {
                TextBlock.Text = value;
            }
        }

        public TextDialog()
        {
            InitializeComponent();
            SizeChanged += new SizeChangedEventHandler(OnSizeChanged);
        }

        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            Debug.WriteLine(e.NewSize.Width);
            if(e.NewSize.Width < 800)
            {
                Scroll.Width = 400;
            }
        }

        private void PrimaryActionButtonClick(ContentDialog dialog, ContentDialogButtonClickEventArgs args)
        {
            Hide();
        }
    }
}
