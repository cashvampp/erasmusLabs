using System.Windows;

namespace BlockLinkingApp
{
    public partial class ConnectionChoiceWindow : Window
    {
        public bool IsTrueSelected { get; private set; }

        public ConnectionChoiceWindow()
        {
            InitializeComponent();
        }

        private void TrueButton_Click(object sender, RoutedEventArgs e)
        {
            IsTrueSelected = true;
            this.DialogResult = true;
            this.Close();
        }

        private void FalseButton_Click(object sender, RoutedEventArgs e)
        {



            IsTrueSelected = false;
            this.DialogResult = true;
            this.Close();
        }
    }
}
