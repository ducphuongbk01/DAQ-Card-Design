using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BTL
{
    public partial class Form1 : Form
    {
        char[] tx_buffer = new char[16];
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Control.CheckForIllegalCrossThreadCalls = false;
            serialPort1.PortName = "COM7";
            serialPort1.Open();
        }

        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            try
            {
                String rcv_data = serialPort1.ReadLine();
                char[] sub_data = rcv_data.ToCharArray();
                if (sub_data[0] == (char)0x02)
                {
                    int dI1 = (int)(sub_data[1]|(sub_data[2]<<7));
                    if ((dI1 & 1) > 0) checkBox3.Checked = true;
                    else checkBox3.Checked = false;
                    if ((dI1 & 2) > 0) checkBox4.Checked = true;
                    else checkBox4.Checked = false;
                    if ((dI1 & 4) > 0) checkBox5.Checked = true;
                    checkBox5.Checked = false;
                    if ((dI1 & 8) > 0) checkBox7.Checked = true;
                    else checkBox7.Checked = false;
                    if ((dI1 & 16) > 0) checkBox8.Checked = true;
                    else checkBox8.Checked = false;
                    if ((dI1 & 32) > 0) checkBox9.Checked = true;
                    else checkBox9.Checked = false;
                    if ((dI1 & 64) > 0) checkBox10.Checked = true;
                    else checkBox10.Checked = false;
                    if ((dI1 & 128) > 0) checkBox11.Checked = true;
                    else checkBox11.Checked = false;
                    int dI2 = (int)(sub_data[3] | (sub_data[4] << 7));
                    if ((dI2 & 1) > 0) checkBox12.Checked = true;
                    else checkBox12.Checked = false;
                    if ((dI2 & 2) > 0) checkBox13.Checked = true;
                    else checkBox13.Checked = false;
                    if ((dI2 & 4) > 0) checkBox14.Checked = true;
                    else checkBox14.Checked = false;
                    if ((dI2 & 8) > 0) checkBox15.Checked = true;
                    else checkBox15.Checked = false;
                    if ((dI2 & 16) > 0) checkBox16.Checked = true;
                    else checkBox16.Checked = false;
                    if ((dI2 & 32) > 0) checkBox17.Checked = true;
                    else checkBox17.Checked = false;
                    if ((dI2 & 64) > 0) checkBox6.Checked = true;
                    else checkBox6.Checked = false;
                    if ((dI2 & 128) > 0) checkBox18.Checked = true;
                    else checkBox18.Checked = false;
                    textBox1.Text = sub_data[5].ToString() + sub_data[6].ToString() + sub_data[7].ToString();
                    textBox2.Text = sub_data[8].ToString() + sub_data[9].ToString() + sub_data[10].ToString();
                    textBox3.Text = sub_data[11].ToString() + sub_data[12].ToString() + sub_data[13].ToString();
                    textBox4.Text = sub_data[14].ToString() + sub_data[15].ToString() + sub_data[16].ToString();
                    textBox8.Text = "";
                    for (int j = 0; j < sub_data.Length; j++)
                    {
                        textBox8.Text += sub_data[j] + ",";
                    }
                }
            }
            catch
            { }
            tx_buffer[0] = (char)0x02;
            tx_buffer[15] = (char)0x03;
            tx_buffer[12] = 'a';
            tx_buffer[13] = 'c';
            tx_buffer[14] = 'k';
            serialPort1.Write(tx_buffer, 0, 16);

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked)
            {
                groupBox6.Visible = false;
                groupBox5.Visible = true;
                checkBox32.Checked = false;
            }
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked)
            {
                groupBox8.Visible = true;
                groupBox9.Visible = false;
                checkBox31.Checked = false;
            }
        }

        private void checkBox32_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox32.Checked)
            {
                groupBox6.Visible = true;
                groupBox5.Visible = false;
                checkBox1.Checked = false;
            }
        }

        private void checkBox31_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox31.Checked)
            {
                groupBox9.Visible = true;
                groupBox8.Visible = false;
                checkBox2.Checked = false;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int dO1 = 0;
            int dO2 = 0;
            tx_buffer[0] = (char)0x02;
            tx_buffer[15] = (char)0x03;
            int mode = 0;
            if (checkBox1.Checked && checkBox2.Checked)
            {
                mode = 0x00;
            }
            else if (checkBox1.Checked && checkBox31.Checked)
            {
                mode = 0x01;
            }
            else if (checkBox2.Checked && checkBox32.Checked)
            {
                mode = 0x02;
            }
            else if (checkBox31.Checked && checkBox32.Checked)
            {
                mode = 0x03;
            }
            tx_buffer[1] = (char)mode;
            //DO1
            if (checkBox32.Checked)
            {
                if (checkBox36.Checked) dO1 |= 1;
                if (checkBox35.Checked) dO1 |= 2;
                if (checkBox34.Checked) dO1 |= 4;
                if (checkBox33.Checked) dO1 |= 8;
                if (checkBox30.Checked) dO1 |= 16;
                if (checkBox29.Checked) dO1 |= 32;
                if (checkBox28.Checked) dO1 |= 64;
                if (checkBox27.Checked) dO1 |= 128;

            }
            if (dO1>127)
            {
                tx_buffer[2] = (char)(dO1 & 127);
                tx_buffer[3] = (char)0x01;
            }
            else
            {
                tx_buffer[2] = (char)dO1;
                tx_buffer[3] = (char)0x00;
            }    
            //DO2
            if (checkBox31.Checked)
            {
                if (checkBox26.Checked) dO2 |= 1;
                if (checkBox25.Checked) dO2 |= 2;
                if (checkBox22.Checked) dO2 |= 4;
                if (checkBox21.Checked) dO2 |= 8;
                if (checkBox20.Checked) dO2 |= 16;
                if (checkBox19.Checked) dO2 |= 32;
                if (checkBox24.Checked) dO2 |= 64;
                if (checkBox23.Checked) dO2 |= 128;
            }
            if (dO2 > 127)
            {
                tx_buffer[4] = (char)(dO2 & 127);
                tx_buffer[5] = (char)0x01;
            }
            else
            {
                tx_buffer[4] = (char)dO2;
                tx_buffer[5] = (char)0x00;
            }

            try
            {
                //DAC1
                char[] dAC1 = textBox5.Text.ToCharArray();
                tx_buffer[6] = dAC1[0];
                tx_buffer[7] = dAC1[1];
                tx_buffer[8] = dAC1[2];
                //DAC2
                char[] dAC2 = textBox6.Text.ToCharArray();
                tx_buffer[9] = dAC2[0];
                tx_buffer[10] = dAC2[1];
                tx_buffer[11] = dAC2[2];
                tx_buffer[12] = 'a';
                tx_buffer[13] = 'c';
                tx_buffer[14] = 'k';
                serialPort1.Write(tx_buffer, 0, 16);
                textBox7.Text = "";
                for (int i = 0; i < tx_buffer.Length; i++)
                {
                    textBox7.Text += ((int)tx_buffer[i]).ToString() + ",";
                }
            }
            catch
            {
                MessageBox.Show("Vui lòng nhập giá trị phù hợp");
            }
        }
    }
}
