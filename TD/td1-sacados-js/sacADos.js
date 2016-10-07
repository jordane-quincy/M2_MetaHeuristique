var sol = {
    x: [],
    slack: []
};
var fileSystem = new ActiveXObject("Scripting.FileSystemObject");
var monfichier = fileSystem.OpenTextFile("100mknap1.dat", 1 ,true);
console.log(monfichier.Read(7)); // imprime: "tutorie"
monFichier.Close();

var mkp = {
    n: 0,
    m: 0

};
