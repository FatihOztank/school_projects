using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace cs408_inLab1
{
    public partial class Form1 : Form
    {

        bool terminating = false;
        bool connected = false;
        Socket clientSocket;

        public Form1()
        {
            Control.CheckForIllegalCrossThreadCalls = false;
            this.FormClosing += new FormClosingEventHandler(Form1_FormClosing);
            InitializeComponent();
        }

        private void button_connect_Click(object sender, EventArgs e)
        {
            clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            string IP = textBox_IP.Text;

            int portNum;
            if (Int32.TryParse(textBox_Port.Text, out portNum))
            {
                try
                {
                    clientSocket.Connect(IP, portNum);
                    button_connect.Enabled = false;
                    connected = true;
                    chat.AppendText("Connected to the server!\n");

                    Thread receiveThread = new Thread(Receive);  // receives the number from server here?
                    receiveThread.Start();

                }
                catch
                {
                    chat.AppendText("Could not connect to the server!\n");
                }
            }
            else
            {
                chat.AppendText("Check the port\n");
            }


        }

        private void Receive()
        {
            bool isDone = false;
            while (connected)
            {
                try
                {
                    Byte[] buffer = new Byte[64];
                    
                    clientSocket.Receive(buffer);

                    string incomingMessage = Encoding.Default.GetString(buffer);
                    incomingMessage = incomingMessage.Substring(0, incomingMessage.IndexOf("\0"));

                    chat.AppendText("Server: " + incomingMessage + "\n");
                    if (isDone)
                    {
                        clientSocket.Close();
                    }

                    int sum = 0; // sum digits
                    foreach (char digit in incomingMessage)
                    {
                        sum += (digit - '0'); // convert char to integer
                    }

                    string sum_str = sum.ToString(); // convert integer to string

                    string payload = sum_str + " " + textBox_Name.Text;
                    if (payload != "" && payload.Length <= 64)
                    {
                        Byte[] bufferSend = new Byte[64];
                        bufferSend = Encoding.Default.GetBytes(payload);
                        clientSocket.Send(bufferSend);
                        chat.AppendText("Message sent: " + payload + "\n");
                        isDone = true;
                    }


                }
                catch
                {
                    if (!terminating)
                    {
                        chat.AppendText("The server has disconnected\n");
                        button_connect.Enabled = true;
                    }

                    clientSocket.Close();
                    connected = false;
                }

            }
        }




        private void Form1_FormClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            connected = false;
            terminating = true;
            Environment.Exit(0);
        }
    }
}
