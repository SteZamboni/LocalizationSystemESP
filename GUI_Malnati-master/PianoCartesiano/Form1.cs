using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;
using MySql.Data.MySqlClient;


namespace PianoCartesiano
{

    public partial class Form1 : Form
    {
        //private Color yellow;
        Timer t = new Timer();

        double[] coorX = new double[10];
        double[] coorY = new double[10];

        double[] pointsX = new double[256];
        double[] pointsY = new double[256];
        string[] Mac = new string[256];
        string[] Fingerprint = new string[256];


        MySql.Data.MySqlClient.MySqlConnection connect;
        int NumberOfESP = 0;
        int NumberOfPackets = 0;

        public Form1()
        {
            InitializeComponent();
            trackBar1.Visible = false;
            dateTimePicker1.Value = DateTime.Now;
            dateTimePicker2.Value = DateTime.Now;
        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {

            this.trackBar1.Scroll += new System.EventHandler(this.trackBar1_Scroll);

            t.Interval = 1000;
            t.Tick += new EventHandler(this.t_Tick);
            t.Start();




        }

        private void t_Tick(object sender, EventArgs e)
        {
            int hh = DateTime.Now.Hour;
            int mm = DateTime.Now.Minute;
            int ss = DateTime.Now.Second;

            string time = "";
            if (hh < 10)
            {
                time += "0" + hh;
            }
            else
            {
                time += hh;
            }
            time += ":";
            if (mm < 10)
            {
                time += "0" + mm;
            }
            else
            {
                time += mm;
            }
            time += ":";
            if (ss < 10)
            {
                time += "0" + ss;
            }
            else
            {
                time += ss;
            }

            label7.Text = time;
        }

        private void NumESP()
        {
            try
            {
                if (connect.State == ConnectionState.Open)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine("SELECT COUNT(*) ");
                    sb.Append(" FROM Coordinates");
                    using (MySqlCommand cmd = new MySqlCommand(sb.ToString(), connect))
                    {
                        NumberOfESP = Convert.ToInt32(cmd.ExecuteScalar());
                    }
                }
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        /*private void NumPackets()
        {
            try
            {
                if (connect.State == ConnectionState.Open)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine("SELECT COUNT(*) ");
                    sb.Append(" FROM Points");
                    using (MySqlCommand cmd = new MySqlCommand(sb.ToString(), connect))
                    {
                        NumberOfPackets = Convert.ToInt32(cmd.ExecuteScalar());
                    }
                }
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {
                MessageBox.Show(ex.Message);
            }
        }*/

        private void NewGraph(double time, double[] xx, double[] yy)
        {
            double maxX = coorX[0];
            double maxY = coorY[0];
            for (int i = 1; i < NumberOfESP + 1; i++)
            {
                if (coorX[i] > maxX)
                    maxX = coorX[i];
                if (coorY[i] > maxY)
                    maxY = coorY[i];

            }
            chart1.Series.Clear();
            var ss = new Series();
            ss.Name = label8.Text;
            ss.ChartArea = "ChartArea1";
            ss.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Point;
            ss.Legend = "Legend1";
            ss.XValueType = ChartValueType.Double;
            ss.YValueType = ChartValueType.Double;
            chart1.ChartAreas[0].BackColor = Color.WhiteSmoke;
            chart1.ChartAreas[0].AxisX.Minimum = -0.5;
            chart1.ChartAreas[0].AxisX.Maximum = maxX + 0.5;
            chart1.ChartAreas[0].AxisX.LabelStyle.Interval = 0.5;
            chart1.ChartAreas[0].AxisY.Minimum = -0.5;
            chart1.ChartAreas[0].AxisY.Maximum = maxY + 0.5;
            chart1.ChartAreas[0].AxisY.LabelStyle.Interval = 0.5;
            chart1.ChartAreas[0].AxisX.MinorGrid.Interval = 0.1;
            chart1.ChartAreas[0].AxisY.MinorGrid.Interval = 0.1;
            chart1.ChartAreas[0].AxisX.MinorGrid.Enabled = true;
            chart1.ChartAreas[0].AxisY.MinorGrid.Enabled = true;
            chart1.ChartAreas[0].AxisX.MinorGrid.LineDashStyle = ChartDashStyle.Dot;
            chart1.ChartAreas[0].AxisY.MinorGrid.LineDashStyle = ChartDashStyle.Dot;
            chart1.ChartAreas[0].AxisX.MajorGrid.Interval = 0.5;
            chart1.ChartAreas[0].AxisY.MajorGrid.Interval = 0.5;
            label11.Text = "m";
            label12.Text = "m";
            label12.BackColor = Color.White;
            label11.Font = new Font(chart1.ChartAreas[0].AxisX.LabelStyle.Font.Name, 10);
            label12.Font = new Font(chart1.ChartAreas[0].AxisX.LabelStyle.Font.Name, 10);

            trackBar1.Visible = true;

            this.chart1.Series.Add(ss);
            this.chart1.Series[ss.Name].IsValueShownAsLabel = true;

            for (int i = 0; i < NumberOfESP; i++)
            {
                double x = coorX[i];
                double y = coorY[i];
                DataPoint dp1 = new DataPoint();
                dp1.SetValueXY(x, y);
                dp1.Font = new Font("Arial", 10, FontStyle.Bold);
                dp1.Label = "ESP32";
                ss.Points.Add(dp1);

            }

            for (int j = 0; j < NumberOfPackets; j++)
            {
                double px = pointsX[j];
                double py = pointsY[j];
                DataPoint dp1 = new DataPoint();
                dp1.SetValueXY(px, py);
                dp1.Font = new Font("Arial", 10, FontStyle.Bold);
                dp1.Label = Mac[j];
                ss.Points.Add(dp1);
            }

            chart1.Invalidate();

        }



        private void queeryCoord()
        {
            try
            {
                if (connect.State == ConnectionState.Open)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine("SELECT CoordX, CoordY ");
                    sb.Append(" FROM Coordinates");

                    using (MySqlCommand cmd = new MySqlCommand(sb.ToString(), connect))
                    {
                        using (MySqlDataReader dr = cmd.ExecuteReader())
                        {
                            int i = 0;
                            while (dr.Read())
                            {
                                coorX[i] = dr.GetDouble("CoordX");
                                coorY[i] = dr.GetDouble("CoordY");
                                i++;
                            }
                        }
                    }
                }

            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {

                MessageBox.Show(ex.Message);
            }
        }
        private void queeryPoints()
        {
            double[] pointsX_t = new double[256];
            double[] pointsY_t = new double[256];
            string[] Mac_t = new string[256];
            string[] Fingerprint_t = new string[256];

            try
            {
                if (connect.State == ConnectionState.Open)
                {
                    string tt1 = "";
                    string tt2 = "";
                    string ttshort = "";
                    int slot = trackBar1.Value;
                    DateTime t_before = DateTime.Now.AddMinutes(-1 * slot - 5);
                    DateTime t_now = DateTime.Now.AddMinutes(-1 * slot);
                    tt1 += t_before.Year + "-" + t_before.Month + "-" + t_before.Day + " " + t_before.Hour + ":" + t_before.Minute + ":" + t_before.Second;
                    tt2 += t_now.Year + "-" + t_now.Month + "-" + t_now.Day + " " + t_now.Hour + ":" + t_now.Minute + ":" + t_now.Second;
                    ttshort += t_now.Hour + ":";
                    if (t_now.Minute < 10) ttshort += "0";
                    ttshort += t_now.Minute;

                    label8.Text = ttshort;

                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine("SELECT COUNT(DISTINCT MACsender) ");
                    sb.Append(" FROM Points ");
                    sb.Append(" WHERE Timestamp <= '" + tt2 + "' AND Timestamp >= '" + tt1 + "' ;");

                    using (MySqlCommand cmd = new MySqlCommand(sb.ToString(), connect))
                    {
                        NumberOfPackets = Convert.ToInt32(cmd.ExecuteScalar());
                    }

                    StringBuilder sb1 = new StringBuilder();
                    sb1.AppendLine("SELECT MACsender, Fingerprint, CoordX, CoordY ");
                    sb1.Append(" FROM Points P1 ");
                    sb1.Append(" WHERE Timestamp = (SELECT MAX(Timestamp) FROM Points P2 WHERE P1.MACsender=P2.MACsender AND Timestamp <= '" + tt2 + "' AND Timestamp >= '" + tt1 + "') ORDER BY Timestamp;");



                    using (MySqlCommand cmd = new MySqlCommand(sb1.ToString(), connect))
                    {
                        using (MySqlDataReader dr = cmd.ExecuteReader())
                        {
                            int i = 0;
                            while (dr.Read())
                            {
                                pointsX_t[i] = dr.GetDouble("CoordX");
                                pointsY_t[i] = dr.GetDouble("CoordY");
                                Mac_t[i] = dr.GetString("MACsender");
                                Fingerprint_t[i] = dr.GetString("Fingerprint");

                                i++;
                            }
                        }
                    }
                }

            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {

                MessageBox.Show(ex.Message);
            }

            int k, j, num, flag = 0;
            num = 0;
            for (j = 0; j < NumberOfPackets; j++)
            {
                if (Mac_t[j][2] != '2' && Mac_t[j][2] != '3' && Mac_t[j][2] != '6' && Mac_t[j][2] != '7' && Mac_t[j][2] != 'a' && Mac_t[j][2] != 'b' && Mac_t[j][2] != 'e' && Mac_t[j][2] != 'f')
                {
                    //real MAC
                    pointsX[num] = pointsX_t[j];
                    pointsY[num] = pointsY_t[j];
                    Mac[num] = Mac_t[j];
                    Fingerprint[num] = Fingerprint_t[j];

                    num++;
                }
                else
                {
                    for (k = 0; k < num; k++)
                    {
                        if (Fingerprint[k] == Fingerprint_t[j])
                        {
                            //Check position
                            double dist = Math.Pow(pointsX_t[j] - pointsX[k], 2) + Math.Pow(pointsY_t[j] - pointsY[k], 2);
                            if (dist < 0.25)
                            {
                                pointsX[k] = pointsX_t[j];
                                pointsY[k] = pointsY_t[j];

                                flag = 1;
                            }
                        }

                    }
                    if (flag != 1)
                    {
                        pointsX[num] = pointsX_t[j];
                        pointsY[num] = pointsY_t[j];
                        Mac[num] = "MAC nascosto";
                        Fingerprint[num] = Fingerprint_t[j];

                        flag = 0;
                        num++;
                    }
                }
            }
            NumberOfPackets = num;

        }
        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            if (trackBar1.Value.Equals("0"))
            {
                timer1.Tick += new EventHandler(this.timer1_Tick);
                //timer1.Start();
            }
            else
            {
                label1.Text = trackBar1.Value.ToString() + " mins ago";

                queeryCoord(); //save coordinates of ESP
                NumESP(); //calculate the number of ESP

                queeryPoints();

                NewGraph(trackBar1.Value, coorX, coorY);

            }


        }

        private void label1_Click(object sender, EventArgs e)
        {

        }


        private void button1_Click(object sender, EventArgs e)
        {
            //
            string MyconnetString;
            MyconnetString = "server=" + textBox1.Text + ";port=" + textBox2.Text + ";Database=" + textBox3.Text + ";uid=" + textBox4.Text + ";pwd=" + textBox5.Text + "; ";

            try
            {
                connect = new MySql.Data.MySqlClient.MySqlConnection();
                connect.ConnectionString = MyconnetString;
                connect.Open();
                if (connect.State == ConnectionState.Open)
                {
                    button1.ForeColor = Color.Green;
                    button2.ForeColor = Color.Empty;
                    MessageBox.Show("Connection success");
                }
                timer1.Interval = 30000;
                timer1.Start();
                label1.Text = Convert.ToString(DateTime.Now);

                queeryCoord(); //save coordinates of ESP
                NumESP(); //calculate the number of ESP

                queeryPoints();

                NewGraph(trackBar1.Value, coorX, coorY);
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {

                MessageBox.Show(ex.Message);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                if (connect.State == ConnectionState.Open)
                {
                    connect.Close();
                    chart1.Series.Clear(); //clean the screen
                    button1.ForeColor = Color.Empty;
                    button2.ForeColor = Color.Green;
                    MessageBox.Show("Connection closed");
                    timer1.Stop();
                }

            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {

                MessageBox.Show(ex.Message);
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            label1.Text = Convert.ToString(DateTime.Now);

            queeryCoord(); //save coordinates of ESP
            NumESP(); //calculate the number of ESP

            queeryPoints();

            NewGraph(trackBar1.Value, coorX, coorY);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            string Fingerprint = "";
            string MacMajor = "";
            int ContFingerprint = 0;
            int flag = 0;
            try
            {

                string tt1 = "";
                string tt2 = "";
                DateTime dt_start = dateTimePicker1.Value;
                DateTime dt_end = dateTimePicker2.Value;
                tt1 += dt_start.Year + "-" + dt_start.Month + "-" + dt_start.Day + " 00:00:00";
                tt2 += dt_end.Year + "-" + dt_end.Month + "-" + dt_end.Day + " 23:59:59";
                if (connect.State == ConnectionState.Open)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine("SELECT fgp, MAX(cnt) as cont FROM (SELECT Fingerprint as fgp, COUNT(*) as cnt ");
                    sb.Append(" FROM Points ");
                    sb.Append(" WHERE Timestamp <= '" + tt2 + "' AND Timestamp >= '" + tt1 + "' ");
                    sb.AppendLine("GROUP BY Fingerprint) AS t; ");

                    using (MySqlCommand cmd = new MySqlCommand(sb.ToString(), connect))
                    {
                        using (MySqlDataReader dr = cmd.ExecuteReader())
                        {
                            while (dr.Read())
                            {
                                if (dr["fgp"] != DBNull.Value)
                                {
                                    Fingerprint = dr.GetString("fgp");
                                    ContFingerprint = dr.GetInt32("cont");
                                }
                                else flag = 1;
                            }
                        }
                    }
                    if (flag == 0)
                    {
                        StringBuilder sb1 = new StringBuilder();
                        sb1.AppendLine("SELECT MACsender ");
                        sb1.Append(" FROM Points P1 ");
                        sb1.Append(value: " WHERE Fingerprint = '" + Fingerprint + "' AND Timestamp <= '" + tt2 + "' AND Timestamp >= '" + tt1 + "';");

                        using (MySqlCommand cmd = new MySqlCommand(sb1.ToString(), connect))
                        {
                            using (MySqlDataReader dr = cmd.ExecuteReader())
                            {
                                while (dr.Read())
                                {
                                    MacMajor = dr.GetString("MACsender");
                                }
                            }
                        }
                    }
                }

            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {

                MessageBox.Show(ex.Message);
            }
            if (flag == 0)
            {
                if (MacMajor[2] != '2' && MacMajor[2] != '3' && MacMajor[2] != '6' && MacMajor[2] != '7' && MacMajor[2] != 'a' && MacMajor[2] != 'b' && MacMajor[2] != 'e' && MacMajor[2] != 'f')
                {
                    //real MAC
                    label10.Text = "Most Active Device: " + MacMajor + " with " + ContFingerprint + " packets";
                }
                else
                {
                    label10.Text = "Most Active Device: MACnascosto with " + ContFingerprint + " packets";
                }
            } else
            {
                label10.Text = "No active devices in the time interval selected";

            }
        }
    }
}
