using LabBackend.Utils.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace LabBackend.Utils
{
    public class Md5Manager : IIdentifiable
    {
        Random rand = new Random();
        public string generateID()
        {
            DateTime currentTime = DateTime.UtcNow;
            double randomValue = rand.NextDouble();
            string uniqueData = $"{currentTime.Ticks}{randomValue}{Guid.NewGuid()}";

            using (MD5 md5Hasher = MD5.Create())
            {
                byte[] data = md5Hasher.ComputeHash(Encoding.Default.GetBytes(uniqueData));

                StringBuilder sBuilder = new StringBuilder();
                for (int i = 0; i < data.Length; i++)
                {
                    sBuilder.Append(data[i].ToString("x2"));
                }

                return sBuilder.ToString();
            }
        }
    }
}
