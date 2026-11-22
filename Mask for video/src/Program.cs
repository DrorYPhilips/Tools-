using System;
using System.Windows.Forms;

namespace MaskVideo
{
    internal static class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // A file path on the command line (or a video dropped on the exe) is opened at start.
            string file = args.Length > 0 ? args[0] : null;
            Application.Run(new MainForm(file));
        }
    }
}
