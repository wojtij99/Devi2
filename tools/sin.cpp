#include "sin.hpp"

/*
struct Lic
{
    public string code;
    public string name;
    public string adress;
    public string zipCode;
    public string NIP;
}
    Lic LicenseV2(string path)
    {
        Lic result;
        result.code = "0";
        result.name = "0";
        result.adress = "0";
        result.zipCode = "0";
        result.NIP = "0";

        #region License
        if (!File.Exists(path)) { MessageBox.Show("Klucz nie został podany", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error); Close(); return result; }

        string Code = File.ReadAllText(path);
        result.code = Code;

        string Stage1 = "";
        foreach (char _char in Code)
        {
            Stage1 = _char + Stage1;
        }

        string Stage2 = "";
        bool cut = false;
        foreach (char _char in Stage1)
        {
            if (cut) cut = false;
            else { Stage2 += _char; cut = true; }
        }
        string[] arr1;

        string Finish = ConvertHex2(Stage2);

        try
        {
            arr1 = Finish.Split('|');
        }
        catch
        {
            MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); 
            return result;
        }

        if (arr1.Length != 4) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }

        string[] arr2;
        try
        {
            arr2 = arr1[2].Split('-');
        }
        catch
        {
            MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return result;
        }

        if (arr2.Length != 3) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }

        if (!CanParse_S(arr2[0])) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }
        if (arr2[0].Length != 2) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }
        if (!CanParse_S(arr2[1])) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }
        if (arr2[1].Length != 3) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }
        if (!CanParse_S(arr1[3])) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }
        if (arr1[3].Length != 10) { MessageBox.Show("Nieprawidłowy klucz!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return result; }


        result.name = arr1[0];
        result.adress = arr1[1];
        result.zipCode = arr2[0] + "-" + arr2[1] + " " + arr2[2];
        result.NIP = arr1[3];

        return result;
        #endregion
    }

    private void btnGenerate_Click(object sender, EventArgs e)
    {
        string[] arr2;
        try
        {
            arr2 = txtZipCode.Text.Split('-');
        }
        catch
        {
            MessageBox.Show("Nieprawidłowy format kodu pocztowego!\n\r Prawidłowy format to: xx-xxx-Miasto", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return;
        }

        if (arr2.Length != 3) { MessageBox.Show("Nieprawidłowy format kodu pocztowego!\n\r Prawidłowy format to: xx-xxx-Miasto", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }

        if (!CanParse(arr2[0])) { MessageBox.Show("Kod pocztowy musi być liczbą!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }
        if (arr2[0].Length != 2) { MessageBox.Show("Nieprawidłowy format kodu pocztowego!\n\r Prawidłowy format to: xx-xxx-Miasto", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }
        if (!CanParse(arr2[1])) { MessageBox.Show("Kod pocztowy musi być liczbą!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }
        if (arr2[1].Length != 3) { MessageBox.Show("Nieprawidłowy format kodu pocztowego!\n\r Prawidłowy format to: xx-xxx-Miasto", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }
        if (!CanParse(txtNIP.Text)) { MessageBox.Show("NIP musi być liczbą!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }
        if (txtNIP.Text.Length != 10) { MessageBox.Show("NIP musi mieć 10 cyfr!", "Błąd", MessageBoxButtons.OK, MessageBoxIcon.Error); return; }

        DialogResult result;
        result = MessageBox.Show("Plik licencji już istnieje!\n\r Czy chcesz nadpisać plik?", "Ostrzeżenie", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
        if (result == DialogResult.No) return; 

        var rand = new Random();
        string Data = txtName.Text + "|" + txtAdress.Text + "|" + txtZipCode.Text + "|" + txtNIP.Text;
        string[] Key = new string[] { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };

        char[] charValues = Data.ToCharArray();
        string hexOutput = "";
        foreach (char _eachChar in charValues)
        {
            int value = Convert.ToInt32(_eachChar);
            string temp = String.Format("{0:X}", value);
            if (temp.Length == 2) temp = "0" + temp;
            hexOutput += temp;
        }

        Console.WriteLine(hexOutput);

        string Convert1 = "";
        foreach (char _char in hexOutput)
        {
            Convert1 += _char;
            Convert1 += Key[rand.Next(16)];
        }

        Console.WriteLine(Convert1);

        string Lic = "";
        foreach (char _char in Convert1)
        {
            Lic = _char + Lic;
        }

        Console.WriteLine(Lic);
        txtProductKey.Text = Lic;

        using (FileStream fs = File.Create(@"Key.Lic"))
        {
            AddText(fs, Lic);
        }

        MessageBox.Show("Wygenerowano klucz", "Sukces", MessageBoxButtons.OK, MessageBoxIcon.Information);
    }
*/

