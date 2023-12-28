using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
namespace WindowsFormsApp1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        string[] baurate = { "1200", "2400", "4800", "9600", "19200","115200" };
        byte[] writebuff = new byte[8];
        byte[] readbuff = new byte[8];
        


        private void Form1_Load(object sender, EventArgs e)
        {   
            string[] listnamecom = SerialPort.GetPortNames();
            listcom.Items.AddRange(listnamecom);
            listbaurate.Items.AddRange(baurate);
            dataGridView1.Columns.Add("Value", "V");
            dataGridView1.Columns.Add("Time", "Time");
            
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (listcom.Text == "" || listbaurate.Text == "")
            {
                MessageBox.Show("chua chon con cong Com hoac baurate");
            }
            else if (serialPort1.IsOpen == true)
            {
                serialPort1.Close();
                button1.Text = "connect";
            }
            else if (serialPort1.IsOpen == false)
            {
                serialPort1.PortName = listcom.Text;
                serialPort1.BaudRate = int.Parse(listbaurate.Text);
                serialPort1.Open();
                button1.Text = "disconnect";
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (textBox1.Text =="")
            {
                MessageBox.Show("chua nhap khoi luong mau");
            }
            else
            {
                /*
                int tmp = int.Parse(textBox1.Text);
                int tmp1 = int.Parse(textBox3.Text);
                writebuff[0] = 0x40;
                writebuff[1] = (byte)(tmp & 0xff);
                writebuff[2] = (byte)((tmp >> 8) & 0xff);
                writebuff[3] = (byte)(tmp1 & 0xff);
                writebuff[4] = (byte)((tmp1 >> 8) & 0xff);
                writebuff[5] = 0x04;
                writebuff[6] = 0x0;
                writebuff[7] = 0xfe;

                serialPort1.Write(writebuff, 0, writebuff.Length);
                */
                int tmp = int.Parse(textBox1.Text);
                writebuff[0] = 0xfe;
                writebuff[1] = 0x00;
                writebuff[2] = 0x04;
                writebuff[3] = 0x00;
                writebuff[4] = 0x01;
                writebuff[5] = (byte)(tmp & 0xff);
                writebuff[6] = (byte)((tmp >> 8) & 0xff);
                writebuff[7] = 0x40;

                serialPort1.Write(writebuff, 0, writebuff.Length);
            }
            
        }
         
        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)

        {
            int V = 0;
            int C = 0;
            int offset = 0;
            int tile = 0;
            int bytesToRead = serialPort1.BytesToRead;
            serialPort1.Read(readbuff, 0, bytesToRead);
            if (readbuff[0] == 0xff && readbuff[7]== 0x40)
            {
                if (readbuff[1] == 0x00)
                {
                    V = V | readbuff[4];
                    V = V << 8;
                    V = V | readbuff[3];
                    V = V & 0xffff;
                    if ((V & (1 << 15)) != 0)
                    {
                        // Bit 15 bằng 1, lấy bù 2
                        V = ~V + 1;
                    }
                    
                    C = C | readbuff[6];
                    C = C << 8;
                    C = C | readbuff[5];
                    //C = C & 0xffff;
                    if ((C & (1 << 15)) != 0)
                    {
                        // Bit 15 bằng 1, lấy bù 2
                        C = ~C + 1;
                        C = C & 0xffff;
                        
                    }

                    Invoke(new Action(() =>
                    {
                        textBox2.Clear();
                        textBox4.Clear();
                        textBox2.AppendText(C.ToString());
                        dataGridView1.Rows.Add(C.ToString(), DateTime.Now.ToString("HH:mm:ss"));
                        
                        textBox4.AppendText(V.ToString());
                    }));
                }
                if (readbuff[1]==0x01)
                {
                    offset = offset | readbuff[6];
                    offset = offset << 8;
                    offset = offset | readbuff[5];
                    offset = offset << 8;
                    offset = offset | readbuff[4];
                    offset = offset << 8;
                    offset = offset | readbuff[3];
                    //textBox5.Clear();
                    //textBox5.AppendText(offset.ToString());
                    Invoke(new Action(() =>
                    {
                        textBox5.Clear();
                        textBox5.AppendText(offset.ToString());
                        

                    }));
                }    
                if (readbuff[1]==0x02)
                {
                    tile= tile| readbuff[4];
                    tile = tile << 8;
                    tile= tile | readbuff[3];
                    tile = tile & 0xffff;
                    if ((tile & (1 << 15)) != 0)
                    {
                        // Bit 15 bằng 1, lấy bù 2
                        tile = ~tile + 1;
                    }
                    //textBox3.Clear();
                    //textBox3.AppendText(tile.ToString());
                    Invoke(new Action(() =>
                    {
                        textBox3.Clear();
                        textBox3.AppendText(tile.ToString());
                        
                    }));
                }    

            }
            else
            {
                Console.WriteLine("error frame");
                MessageBox.Show("Error Frame");
            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {

        }

        private void label9_Click(object sender, EventArgs e)
        {

        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {

        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }
        

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void label10_Click(object sender, EventArgs e)
        {

        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
